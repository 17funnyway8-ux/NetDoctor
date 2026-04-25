#include "ConfigManager.h"
#include "Utils.h"
#include <windows.h>
#include <fstream>

bool ConfigManager::SaveDefaultIfNotExists(const std::wstring& path) {
    DWORD attr = GetFileAttributesW(path.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) return true;
    std::ofstream out(Utils::WideToUtf8(path), std::ios::binary);
    if (!out) return false;
    out << R"ini([General]
CheckIntervalSeconds=30
TimeoutMilliseconds=3000
EnableTooltip=1
Language=zh-CN

[Threshold]
LatencyGoodMs=100
LatencySlowMs=300
DnsGoodMs=80
DnsSlowMs=200

[DNS]
Enabled=1
Domains=www.baidu.com,github.com,cloudflare.com

[CN]
Enabled=1
Targets=Baidu|https://www.baidu.com,QQ|https://www.qq.com

[International]
Enabled=1
Targets=GitHub|https://github.com,Cloudflare|https://www.cloudflare.com

[Developer]
Enabled=0
Targets=GitHub|https://github.com,GitHubRaw|https://raw.githubusercontent.com,npm|https://registry.npmjs.org,PyPI|https://pypi.org,Docker|https://hub.docker.com

[Proxy]
Enabled=1
DetectSystemProxy=1
Ports=127.0.0.1:7890,127.0.0.1:7897,127.0.0.1:1080,127.0.0.1:20171

[CustomSites]
Enabled=0
Targets=NAS|http://192.168.1.10:5000,Router|http://192.168.1.1,Blog|https://example.com

[PublicIP]
Enabled=0
CheckIntervalSeconds=600
Providers=https://api.ipify.org,https://ifconfig.me/ip
)ini";
    return true;
}

bool ConfigManager::Load(const std::wstring& path) {
    m_path = path; SaveDefaultIfNotExists(path);
    m_check_interval_seconds = ReadInt(L"General", L"CheckIntervalSeconds", 30);
    m_timeout_ms = ReadInt(L"General", L"TimeoutMilliseconds", 3000);
    m_thresholds.latency_good_ms = ReadInt(L"Threshold", L"LatencyGoodMs", 100);
    m_thresholds.latency_slow_ms = ReadInt(L"Threshold", L"LatencySlowMs", 300);
    m_thresholds.dns_good_ms = ReadInt(L"Threshold", L"DnsGoodMs", 80);
    m_thresholds.dns_slow_ms = ReadInt(L"Threshold", L"DnsSlowMs", 200);
    m_dns_domains = Utils::Split(ReadString(L"DNS", L"Domains", L"www.baidu.com,github.com,cloudflare.com"), L',');
    m_cn_targets = ParseTargets(ReadString(L"CN", L"Targets", L"Baidu|https://www.baidu.com,QQ|https://www.qq.com"));
    m_intl_targets = ParseTargets(ReadString(L"International", L"Targets", L"GitHub|https://github.com,Cloudflare|https://www.cloudflare.com"));
    m_dev_enabled = ReadInt(L"Developer", L"Enabled", 0) != 0;
    m_dev_targets = ParseTargets(ReadString(L"Developer", L"Targets", L"GitHub|https://github.com,GitHubRaw|https://raw.githubusercontent.com,npm|https://registry.npmjs.org,PyPI|https://pypi.org,Docker|https://hub.docker.com"));
    m_custom_enabled = ReadInt(L"CustomSites", L"Enabled", 0) != 0;
    m_custom_targets = ParseTargets(ReadString(L"CustomSites", L"Targets", L"NAS|http://192.168.1.10:5000,Router|http://192.168.1.1,Blog|https://example.com"));
    m_public_ip_enabled = ReadInt(L"PublicIP", L"Enabled", 0) != 0;
    m_public_ip_check_interval_seconds = ReadInt(L"PublicIP", L"CheckIntervalSeconds", 600);
    m_public_ip_providers = Utils::Split(ReadString(L"PublicIP", L"Providers", L"https://api.ipify.org,https://ifconfig.me/ip"), L',');
    m_proxy_enabled = ReadInt(L"Proxy", L"Enabled", 1) != 0;
    m_detect_system_proxy = ReadInt(L"Proxy", L"DetectSystemProxy", 1) != 0;
    m_proxy_ports = ParseEndpoints(ReadString(L"Proxy", L"Ports", L"127.0.0.1:7890,127.0.0.1:7897,127.0.0.1:1080,127.0.0.1:20171"));
    return true;
}
std::wstring ConfigManager::ReadString(const wchar_t* section, const wchar_t* key, const wchar_t* def) const {
    wchar_t buf[4096]{}; GetPrivateProfileStringW(section, key, def, buf, 4096, m_path.c_str()); return buf;
}
int ConfigManager::ReadInt(const wchar_t* section, const wchar_t* key, int def) const { return GetPrivateProfileIntW(section, key, def, m_path.c_str()); }
std::vector<TargetConfig> ConfigManager::ParseTargets(const std::wstring& raw) const {
    std::vector<TargetConfig> targets;
    for (const auto& item : Utils::Split(raw, L',')) {
        auto p = item.find(L'|');
        if (p == std::wstring::npos) targets.push_back({ item, item, true });
        else targets.push_back({ Utils::Trim(item.substr(0, p)), Utils::Trim(item.substr(p + 1)), true });
    }
    return targets;
}
std::vector<EndpointConfig> ConfigManager::ParseEndpoints(const std::wstring& raw) const {
    std::vector<EndpointConfig> endpoints;
    for (const auto& item : Utils::Split(raw, L',')) {
        auto p = item.find_last_of(L':'); if (p == std::wstring::npos) continue;
        EndpointConfig e; e.host = Utils::Trim(item.substr(0, p)); e.port = static_cast<unsigned short>(_wtoi(item.substr(p + 1).c_str()));
        if (!e.host.empty() && e.port) endpoints.push_back(e);
    }
    return endpoints;
}
