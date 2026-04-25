#include "NetDoctorPlugin.h"
#include "NetDoctorItems.h"
#include "Utils.h"
#include <windows.h>
#include <sstream>
#include <utility>

CNetDoctorPlugin& CNetDoctorPlugin::Instance() { static CNetDoctorPlugin inst; return inst; }

CNetDoctorPlugin::~CNetDoctorPlugin() {
    m_shutdown = true;
    if (m_worker_thread.joinable()) m_worker_thread.join();
}

CNetDoctorPlugin::CNetDoctorPlugin() {
    m_items.emplace_back(std::make_unique<CNetSummaryItem>(*this));
    m_items.emplace_back(std::make_unique<CDnsStatusItem>(*this));
    m_items.emplace_back(std::make_unique<CCnStatusItem>(*this));
    m_items.emplace_back(std::make_unique<CIntlStatusItem>(*this));
    m_items.emplace_back(std::make_unique<CProxyStatusItem>(*this));
    m_items.emplace_back(std::make_unique<CPublicIpItem>(*this));
    m_items.emplace_back(std::make_unique<CDeveloperSitesItem>(*this));
    m_items.emplace_back(std::make_unique<CCustomSitesItem>(*this));
    LoadConfig();
}
IPluginItem* CNetDoctorPlugin::GetItem(int index) { return (index >= 0 && index < (int)m_items.size()) ? m_items[index].get() : nullptr; }
void CNetDoctorPlugin::DataRequired() { UpdateDataIfNeeded(false); }
ITMPlugin::OptionReturn CNetDoctorPlugin::ShowOptionsDialog(void*) { return OR_OPTION_NOT_PROVIDED; }
const wchar_t* CNetDoctorPlugin::GetInfo(PluginInfoIndex index) {
    switch (index) {
    case TMI_NAME: return L"NetDoctor";
    case TMI_DESCRIPTION: return L"网络质量诊断插件：DNS、国内/国际连通性、代理状态。";
    case TMI_AUTHOR: return L"clawclawclaw";
    case TMI_COPYRIGHT: return L"MIT";
    case TMI_VERSION: return L"0.5.0";
    case TMI_URL: return L"";
    default: return L"";
    }
}
const wchar_t* CNetDoctorPlugin::GetTooltipInfo() {
    thread_local std::wstring tooltip_snapshot;
    std::lock_guard<std::mutex> lock(m_state_mutex);
    tooltip_snapshot = m_tooltip;
    return tooltip_snapshot.c_str();
}
NetDoctorState CNetDoctorPlugin::GetStateSnapshot() const { std::lock_guard<std::mutex> lock(m_state_mutex); return m_state; }
void CNetDoctorPlugin::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) {
    if (index == EI_CONFIG_DIR && data && *data) { m_config_dir = data; LoadConfig(); }
}
void CNetDoctorPlugin::LoadConfig() {
    std::wstring base = m_config_dir.empty() ? Utils::GetModuleDirectory() : m_config_dir;
    std::wstring dir = Utils::JoinPath(base, L"NetDoctor");
    Utils::EnsureDirectory(dir);
    m_config.Load(Utils::JoinPath(dir, L"NetDoctor.ini"));
}
void CNetDoctorPlugin::UpdateDataIfNeeded(bool force) {
    auto now = std::chrono::steady_clock::now();
    if (!force && m_last_check.time_since_epoch().count() != 0 &&
        std::chrono::duration_cast<std::chrono::seconds>(now - m_last_check).count() < m_config.CheckIntervalSeconds()) return;
    if (m_checking.load()) return;
    m_last_check = now;
    StartWorkerCheck();
}

void CNetDoctorPlugin::StartWorkerCheck() {
    bool expected = false;
    if (!m_checking.compare_exchange_strong(expected, true)) return;
    if (m_worker_thread.joinable()) m_worker_thread.join();
    ConfigManager config = m_config;
    m_worker_thread = std::thread([this, config]() mutable {
        try {
            NetworkChecker checker;
            DiagnosisEngine diagnosis;
            NetDoctorState state = checker.CheckAll(config);
            diagnosis.Analyze(state, config);
            ApplyWorkerResult(std::move(state));
        } catch (...) {
            NetDoctorState state;
            state.last_error = L"Unknown exception during check";
            state.diagnosis.summary_text = L"CHECK ERR";
            ApplyWorkerResult(std::move(state));
        }
        if (!m_shutdown.load()) m_has_checked = true;
        m_checking = false;
    });
}

void CNetDoctorPlugin::ApplyWorkerResult(NetDoctorState&& state) {
    std::lock_guard<std::mutex> lock(m_state_mutex);
    m_state = std::move(state);
    BuildTooltip();
}
static void AppendHttp(std::wstringstream& ss, const wchar_t* title, const std::vector<HttpCheckResult>& rs) {
    ss << title << L":\n";
    for (auto& r : rs) ss << L"  " << r.name << L": " << (r.success ? L"OK " : L"BAD ") << (r.latency_ms >= 0 ? std::to_wstring(r.latency_ms) + L"ms" : L"") << (r.status_code ? L" HTTP " + std::to_wstring(r.status_code) : L"") << (r.error_message.empty() ? L"" : L" " + r.error_message) << L"\n";
}
void CNetDoctorPlugin::BuildTooltip() {
    std::wstringstream ss;
    ss << L"NetDoctor\n\nDiagnosis:\n  " << m_state.diagnosis.summary_text << L"\n\n";
    ss << L"DNS:\n";
    for (auto& r : m_state.dns_results) ss << L"  " << r.domain << L": " << (r.success ? L"OK " : L"BAD ") << r.latency_ms << L"ms" << (r.error_message.empty() ? L"" : L" " + r.error_message) << L"\n";
    ss << L"\n"; AppendHttp(ss, L"CN", m_state.cn_results); ss << L"\n"; AppendHttp(ss, L"International", m_state.intl_results);
    if (!m_state.dev_results.empty()) { ss << L"\n"; AppendHttp(ss, L"Developer", m_state.dev_results); }
    if (!m_state.custom_results.empty()) { ss << L"\n"; AppendHttp(ss, L"Custom Sites", m_state.custom_results); }
    ss << L"\nProxy:\n  System: " << (m_state.proxy_status.system_proxy_enabled ? L"ON " : L"OFF ") << m_state.proxy_status.proxy_server << L"\n";
    ss << L"  Local: " << (m_state.proxy_status.local_proxy_detected ? L"ON " : L"OFF ") << m_state.proxy_status.detected_proxy_address << L"\n";
    ss << L"\nPublic IP:\n  " << (m_state.public_ip.enabled ? (m_state.public_ip.success ? m_state.public_ip.ip : L"BAD") : L"Disabled") << L"\n";
    if (!m_state.public_ip.provider.empty()) ss << L"  Provider: " << m_state.public_ip.provider << L"\n";
    if (!m_state.public_ip.error_message.empty()) ss << L"  Error: " << m_state.public_ip.error_message << L"\n";
    ss << L"\nLast Check:\n  " << Utils::FormatTimeNow();
    m_tooltip = ss.str();
}
