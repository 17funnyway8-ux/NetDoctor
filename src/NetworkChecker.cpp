#include "NetworkChecker.h"
#include <chrono>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

NetDoctorState NetworkChecker::CheckAll(const ConfigManager& config) {
    WSADATA wsa{}; WSAStartup(MAKEWORD(2,2), &wsa);
    NetDoctorState s;
    if (config.DnsEnabled()) s.dns_results = m_dns.Check(config.DnsDomains());
    if (config.CnEnabled()) s.cn_results = m_http.Check(config.CnTargets(), config.TimeoutMilliseconds());
    if (config.IntlEnabled()) s.intl_results = m_http.Check(config.IntlTargets(), config.TimeoutMilliseconds());
    if (config.DeveloperEnabled()) s.dev_results = m_http.Check(config.DeveloperTargets(), config.TimeoutMilliseconds());
    if (config.CustomSitesEnabled()) s.custom_results = m_http.Check(config.CustomTargets(), config.TimeoutMilliseconds());
    if (config.PingEnabled()) s.ping_results = m_ping.Check(config.PingHosts(), config.PingCount(), config.TimeoutMilliseconds());
    if (config.AiEnabled()) s.ai_results = m_http.Check(config.AiTargets(), config.TimeoutMilliseconds());
    if (config.HomeEnabled()) s.home_results = m_http.Check(config.HomeTargets(), config.TimeoutMilliseconds());
    if (config.ProxyEnabled()) s.proxy_status = m_proxy.Check(config.ProxyPorts(), config.DetectSystemProxy(), config.TimeoutMilliseconds());
    if (config.PublicIpEnabled()) s.public_ip = m_public_ip.Check(config.PublicIpProviders(), config.TimeoutMilliseconds());
    s.last_update = std::chrono::system_clock::now();
    WSACleanup();
    return s;
}
