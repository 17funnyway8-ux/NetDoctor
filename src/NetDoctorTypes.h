#pragma once
#include <chrono>
#include <string>
#include <vector>

enum class CheckStatus { Unknown, Ok, Slow, Warning, Bad };

struct TargetConfig { std::wstring name; std::wstring url; bool enabled{true}; };
struct EndpointConfig { std::wstring host; unsigned short port{}; bool enabled{true}; };

struct ThresholdConfig {
    int latency_good_ms{100};
    int latency_slow_ms{300};
    int dns_good_ms{80};
    int dns_slow_ms{200};
};

struct HttpCheckResult {
    std::wstring name;
    std::wstring url;
    bool success{false};
    int status_code{0};
    int latency_ms{-1};
    std::wstring error_message;
};

struct DnsCheckResult {
    std::wstring domain;
    bool success{false};
    int latency_ms{-1};
    std::vector<std::wstring> addresses;
    std::wstring error_message;
};

struct ProxyStatus {
    bool system_proxy_enabled{false};
    std::wstring proxy_server;
    bool auto_config_enabled{false};
    std::wstring auto_config_url;
    bool local_proxy_detected{false};
    std::wstring detected_proxy_address;
    bool proxy_usable{false};
};

struct PublicIpInfo {
    bool enabled{false};
    bool success{false};
    std::wstring ip;
    std::wstring provider;
    std::wstring error_message;
    std::chrono::system_clock::time_point last_update{};
};

struct AreaStatus {
    CheckStatus status{CheckStatus::Unknown};
    int avg_latency_ms{-1};
    int success_count{0};
    int total_count{0};
};

struct DiagnosisResult {
    CheckStatus overall_status{CheckStatus::Unknown};
    std::wstring summary_text{L"CHECK..."};
    std::wstring detail_text;
};

struct NetDoctorState {
    DiagnosisResult diagnosis;
    AreaStatus dns_status;
    AreaStatus cn_status;
    AreaStatus intl_status;
    std::vector<DnsCheckResult> dns_results;
    std::vector<HttpCheckResult> cn_results;
    std::vector<HttpCheckResult> intl_results;
    ProxyStatus proxy_status;
    PublicIpInfo public_ip;
    std::wstring last_error;
    std::chrono::system_clock::time_point last_update{};
};
