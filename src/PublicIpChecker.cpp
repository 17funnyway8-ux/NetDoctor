#include "PublicIpChecker.h"
#include "Utils.h"
#include <windows.h>
#include <winhttp.h>
#include <chrono>
#include <string>
#pragma comment(lib, "Winhttp.lib")

static bool CrackUrl(const std::wstring& url, URL_COMPONENTS& parts, std::wstring& host, std::wstring& path) {
    ZeroMemory(&parts, sizeof(parts)); parts.dwStructSize = sizeof(parts);
    wchar_t host_buf[512]{}; wchar_t path_buf[2048]{};
    parts.lpszHostName = host_buf; parts.dwHostNameLength = 512;
    parts.lpszUrlPath = path_buf; parts.dwUrlPathLength = 2048;
    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &parts)) return false;
    host.assign(parts.lpszHostName, parts.dwHostNameLength);
    path.assign(parts.lpszUrlPath, parts.dwUrlPathLength);
    if (path.empty()) path = L"/";
    return true;
}

PublicIpInfo PublicIpChecker::Check(const std::vector<std::wstring>& providers, int timeout_ms) {
    PublicIpInfo info; info.enabled = true; info.last_update = std::chrono::system_clock::now();
    for (const auto& provider : providers) {
        std::wstring body, err;
        if (FetchText(provider, timeout_ms, body, err)) {
            body = Utils::Trim(body);
            if (!body.empty() && body.size() < 128) {
                info.success = true;
                info.ip = body;
                info.provider = provider;
                return info;
            }
            err = L"Empty or invalid response";
        }
        info.error_message = err.empty() ? L"Provider failed" : err;
    }
    return info;
}

bool PublicIpChecker::FetchText(const std::wstring& url, int timeout_ms, std::wstring& body, std::wstring& error_message) {
    URL_COMPONENTS parts{}; std::wstring host, path;
    if (!CrackUrl(url, parts, host, path)) { error_message = L"Invalid URL"; return false; }
    HINTERNET session = WinHttpOpen(L"NetDoctor/0.5", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) { error_message = L"WinHttpOpen failed"; return false; }
    WinHttpSetTimeouts(session, timeout_ms, timeout_ms, timeout_ms, timeout_ms);
    HINTERNET connect = WinHttpConnect(session, host.c_str(), parts.nPort, 0);
    if (!connect) { error_message = L"WinHttpConnect failed"; WinHttpCloseHandle(session); return false; }
    DWORD flags = (parts.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET req = WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!req) { error_message = L"WinHttpOpenRequest failed"; WinHttpCloseHandle(connect); WinHttpCloseHandle(session); return false; }
    bool ok = false;
    if (WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(req, nullptr)) {
        DWORD status = 0, size = sizeof(status);
        WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &status, &size, nullptr);
        if (status >= 200 && status < 300) {
            std::string bytes;
            DWORD available = 0;
            while (WinHttpQueryDataAvailable(req, &available) && available > 0 && bytes.size() < 4096) {
                std::string chunk(available, '\0'); DWORD read = 0;
                if (!WinHttpReadData(req, chunk.data(), available, &read)) break;
                chunk.resize(read); bytes += chunk;
            }
            body = Utils::Utf8ToWide(bytes);
            ok = true;
        } else {
            error_message = L"HTTP " + std::to_wstring(status);
        }
    } else {
        error_message = L"HTTP request failed: " + std::to_wstring(GetLastError());
    }
    WinHttpCloseHandle(req); WinHttpCloseHandle(connect); WinHttpCloseHandle(session);
    return ok;
}
