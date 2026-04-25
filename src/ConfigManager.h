#pragma once
#include "NetDoctorTypes.h"
#include <string>
#include <vector>

class ConfigManager {
public:
    bool Load(const std::wstring& path);
    bool SaveDefaultIfNotExists(const std::wstring& path);
    const std::wstring& Path() const { return m_path; }
    int CheckIntervalSeconds() const { return m_check_interval_seconds; }
    int TimeoutMilliseconds() const { return m_timeout_ms; }
    const ThresholdConfig& Thresholds() const { return m_thresholds; }
    const std::vector<std::wstring>& DnsDomains() const { return m_dns_domains; }
    const std::vector<TargetConfig>& CnTargets() const { return m_cn_targets; }
    const std::vector<TargetConfig>& IntlTargets() const { return m_intl_targets; }
    const std::vector<TargetConfig>& DeveloperTargets() const { return m_dev_targets; }
    const std::vector<TargetConfig>& CustomTargets() const { return m_custom_targets; }
    const std::vector<std::wstring>& PingHosts() const { return m_ping_hosts; }
    const std::vector<EndpointConfig>& ProxyPorts() const { return m_proxy_ports; }
    const std::vector<std::wstring>& PublicIpProviders() const { return m_public_ip_providers; }
    bool DeveloperEnabled() const { return m_dev_enabled; }
    bool CustomSitesEnabled() const { return m_custom_enabled; }
    bool PingEnabled() const { return m_ping_enabled; }
    int PingCount() const { return m_ping_count; }
    bool PublicIpEnabled() const { return m_public_ip_enabled; }
    int PublicIpCheckIntervalSeconds() const { return m_public_ip_check_interval_seconds; }
    bool ProxyEnabled() const { return m_proxy_enabled; }
    bool DetectSystemProxy() const { return m_detect_system_proxy; }
private:
    std::wstring ReadString(const wchar_t* section, const wchar_t* key, const wchar_t* def) const;
    int ReadInt(const wchar_t* section, const wchar_t* key, int def) const;
    std::vector<TargetConfig> ParseTargets(const std::wstring& raw) const;
    std::vector<EndpointConfig> ParseEndpoints(const std::wstring& raw) const;
private:
    std::wstring m_path;
    int m_check_interval_seconds{30};
    int m_timeout_ms{3000};
    ThresholdConfig m_thresholds;
    std::vector<std::wstring> m_dns_domains;
    std::vector<TargetConfig> m_cn_targets;
    std::vector<TargetConfig> m_intl_targets;
    std::vector<TargetConfig> m_dev_targets;
    std::vector<TargetConfig> m_custom_targets;
    std::vector<std::wstring> m_ping_hosts;
    std::vector<EndpointConfig> m_proxy_ports;
    std::vector<std::wstring> m_public_ip_providers;
    bool m_dev_enabled{false};
    bool m_custom_enabled{false};
    bool m_ping_enabled{false};
    int m_ping_count{4};
    bool m_public_ip_enabled{false};
    int m_public_ip_check_interval_seconds{600};
    bool m_proxy_enabled{true};
    bool m_detect_system_proxy{true};
};
