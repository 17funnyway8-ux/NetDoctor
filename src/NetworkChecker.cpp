#include "NetworkChecker.h"
#include <chrono>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

NetDoctorState NetworkChecker::CheckAll(const ConfigManager& config) {
    WSADATA wsa{}; WSAStartup(MAKEWORD(2,2), &wsa);
    NetDoctorState s;
    s.dns_results = m_dns.Check(config.DnsDomains());
    s.cn_results = m_http.Check(config.CnTargets(), config.TimeoutMilliseconds());
    s.intl_results = m_http.Check(config.IntlTargets(), config.TimeoutMilliseconds());
    if (config.ProxyEnabled()) s.proxy_status = m_proxy.Check(config.ProxyPorts(), config.DetectSystemProxy(), config.TimeoutMilliseconds());
    if (config.PublicIpEnabled()) s.public_ip = m_public_ip.Check(config.PublicIpProviders(), config.TimeoutMilliseconds());
    s.last_update = std::chrono::system_clock::now();
    WSACleanup();
    return s;
}
