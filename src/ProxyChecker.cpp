#include "ProxyChecker.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winhttp.h>
#include <sstream>
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Ws2_32.lib")

ProxyStatus ProxyChecker::Check(const std::vector<EndpointConfig>& ports, bool detect_system_proxy, int timeout_ms) {
    ProxyStatus s;
    if (detect_system_proxy) {
        WINHTTP_CURRENT_USER_IE_PROXY_CONFIG cfg{};
        if (WinHttpGetIEProxyConfigForCurrentUser(&cfg)) {
            s.system_proxy_enabled = cfg.lpszProxy != nullptr;
            if (cfg.lpszProxy) { s.proxy_server = cfg.lpszProxy; GlobalFree(cfg.lpszProxy); }
            s.auto_config_enabled = cfg.lpszAutoConfigUrl != nullptr;
            if (cfg.lpszAutoConfigUrl) { s.auto_config_url = cfg.lpszAutoConfigUrl; GlobalFree(cfg.lpszAutoConfigUrl); }
            if (cfg.lpszProxyBypass) GlobalFree(cfg.lpszProxyBypass);
        }
    }
    for (const auto& ep : ports) {
        if (!ep.enabled) continue;
        if (IsTcpOpen(ep.host, ep.port, timeout_ms > 1000 ? 1000 : timeout_ms)) {
            s.local_proxy_detected = true;
            std::wstringstream ss; ss << ep.host << L":" << ep.port;
            s.detected_proxy_address = ss.str();
            break;
        }
    }
    s.proxy_usable = s.system_proxy_enabled || s.local_proxy_detected;
    return s;
}

bool ProxyChecker::IsTcpOpen(const std::wstring& host, unsigned short port, int timeout_ms) {
    addrinfoW hints{}; hints.ai_family = AF_UNSPEC; hints.ai_socktype = SOCK_STREAM;
    addrinfoW* info = nullptr; std::wstring port_s = std::to_wstring(port);
    if (GetAddrInfoW(host.c_str(), port_s.c_str(), &hints, &info) != 0) return false;
    bool open = false;
    for (auto p = info; p && !open; p = p->ai_next) {
        SOCKET sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == INVALID_SOCKET) continue;
        u_long nb = 1; ioctlsocket(sock, FIONBIO, &nb);
        int rc = connect(sock, p->ai_addr, (int)p->ai_addrlen);
        if (rc == 0) open = true;
        else if (WSAGetLastError() == WSAEWOULDBLOCK) {
            fd_set wfds; FD_ZERO(&wfds); FD_SET(sock, &wfds);
            timeval tv{}; tv.tv_sec = timeout_ms / 1000; tv.tv_usec = (timeout_ms % 1000) * 1000;
            if (select(0, nullptr, &wfds, nullptr, &tv) > 0) {
                int err = 0; int len = sizeof(err); getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len); open = (err == 0);
            }
        }
        closesocket(sock);
    }
    FreeAddrInfoW(info);
    return open;
}
