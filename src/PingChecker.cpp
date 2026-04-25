#include "PingChecker.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <icmpapi.h>
#include <iphlpapi.h>
#include <algorithm>
#include <limits>
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

std::vector<PingResult> PingChecker::Check(const std::vector<std::wstring>& hosts, int count, int timeout_ms) {
    std::vector<PingResult> results;
    for (const auto& h : hosts) results.push_back(CheckOne(h, count, timeout_ms));
    return results;
}

bool PingChecker::ResolveIpv4(const std::wstring& host, unsigned long& ip_addr, std::wstring& error_message) {
    addrinfoW hints{}; hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
    addrinfoW* info = nullptr;
    int rc = GetAddrInfoW(host.c_str(), nullptr, &hints, &info);
    if (rc != 0 || !info) { error_message = L"Resolve failed: " + std::to_wstring(rc); return false; }
    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(info->ai_addr);
    ip_addr = addr->sin_addr.S_un.S_addr;
    FreeAddrInfoW(info);
    return true;
}

PingResult PingChecker::CheckOne(const std::wstring& host, int count, int timeout_ms) {
    PingResult r; r.host = host; r.sent = count;
    unsigned long ip = 0;
    if (!ResolveIpv4(host, ip, r.error_message)) return r;
    HANDLE icmp = IcmpCreateFile();
    if (icmp == INVALID_HANDLE_VALUE) { r.error_message = L"IcmpCreateFile failed"; return r; }
    char send_data[] = "NetDoctor";
    DWORD reply_size = sizeof(ICMP_ECHO_REPLY) + sizeof(send_data) + 64;
    std::vector<char> reply(reply_size);
    int total = 0; int min_ms = std::numeric_limits<int>::max(); int max_ms = -1;
    DWORD per_timeout = timeout_ms > 1000 ? 1000 : timeout_ms;
    for (int i = 0; i < count; ++i) {
        DWORD ret = IcmpSendEcho(icmp, ip, send_data, sizeof(send_data), nullptr, reply.data(), reply_size, per_timeout);
        if (ret > 0) {
            auto* echo = reinterpret_cast<PICMP_ECHO_REPLY>(reply.data());
            if (echo->Status == IP_SUCCESS) {
                int ms = static_cast<int>(echo->RoundTripTime);
                r.received++; total += ms; min_ms = std::min(min_ms, ms); max_ms = std::max(max_ms, ms);
            }
        }
    }
    IcmpCloseHandle(icmp);
    r.success = r.received > 0;
    r.loss_percent = r.sent > 0 ? (100 - (r.received * 100 / r.sent)) : 100;
    if (r.received > 0) {
        r.avg_latency_ms = total / r.received;
        r.min_latency_ms = min_ms;
        r.max_latency_ms = max_ms;
    } else if (r.error_message.empty()) {
        r.error_message = L"No ping replies";
    }
    return r;
}
