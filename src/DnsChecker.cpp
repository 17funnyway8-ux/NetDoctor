#include "DnsChecker.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#pragma comment(lib, "Ws2_32.lib")

std::vector<DnsCheckResult> DnsChecker::Check(const std::vector<std::wstring>& domains) {
    std::vector<DnsCheckResult> results;
    for (const auto& domain : domains) {
        DnsCheckResult r; r.domain = domain;
        auto start = std::chrono::steady_clock::now();
        addrinfoW hints{}; hints.ai_family = AF_UNSPEC; hints.ai_socktype = SOCK_STREAM;
        addrinfoW* info = nullptr;
        int rc = GetAddrInfoW(domain.c_str(), nullptr, &hints, &info);
        auto end = std::chrono::steady_clock::now();
        r.latency_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (rc == 0 && info) {
            r.success = true;
            for (auto p = info; p; p = p->ai_next) {
                wchar_t host[NI_MAXHOST]{};
                if (GetNameInfoW(p->ai_addr, (socklen_t)p->ai_addrlen, host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) == 0) r.addresses.push_back(host);
            }
            FreeAddrInfoW(info);
        } else {
            r.error_message = L"GetAddrInfoW failed: " + std::to_wstring(rc);
        }
        results.push_back(std::move(r));
    }
    return results;
}
