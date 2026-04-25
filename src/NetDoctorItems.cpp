#include "NetDoctorItems.h"
#include "NetDoctorPlugin.h"
#include <sstream>
#include <utility>

CNetDoctorItemBase::CNetDoctorItemBase(CNetDoctorPlugin& plugin, std::wstring name, std::wstring id, std::wstring label, std::wstring sample)
    : m_plugin(plugin), m_name(std::move(name)), m_id(std::move(id)), m_label(std::move(label)), m_sample(std::move(sample)) {}
const wchar_t* CNetDoctorItemBase::GetItemValueText() const { m_value = BuildValue(); return m_value.c_str(); }
CNetSummaryItem::CNetSummaryItem(CNetDoctorPlugin& p) : CNetDoctorItemBase(p, L"网络诊断", L"NetDoctorSummary", L"NET", L"INTL SLOW") {}
CDnsStatusItem::CDnsStatusItem(CNetDoctorPlugin& p) : CNetDoctorItemBase(p, L"DNS 状态", L"NetDoctorDNS", L"DNS", L"DNS 88ms") {}
CCnStatusItem::CCnStatusItem(CNetDoctorPlugin& p) : CNetDoctorItemBase(p, L"国内网络", L"NetDoctorCN", L"CN", L"CN 88ms") {}
CIntlStatusItem::CIntlStatusItem(CNetDoctorPlugin& p) : CNetDoctorItemBase(p, L"国际网络", L"NetDoctorIntl", L"INTL", L"INTL 288ms") {}
CProxyStatusItem::CProxyStatusItem(CNetDoctorPlugin& p) : CNetDoctorItemBase(p, L"代理状态", L"NetDoctorProxy", L"Proxy", L"Proxy ON") {}
CPublicIpItem::CPublicIpItem(CNetDoctorPlugin& p) : CNetDoctorItemBase(p, L"公网出口", L"NetDoctorPublicIP", L"IP", L"IP 1.2.3.4") {}
CDeveloperSitesItem::CDeveloperSitesItem(CNetDoctorPlugin& p) : CNetDoctorItemBase(p, L"开发者站点", L"NetDoctorDevSites", L"DEV", L"DEV 4/5") {}
CCustomSitesItem::CCustomSitesItem(CNetDoctorPlugin& p) : CNetDoctorItemBase(p, L"自定义站点", L"NetDoctorCustomSites", L"SITE", L"SITE 3/4") {}
std::wstring CNetSummaryItem::BuildValue() const { return m_plugin.GetStateSnapshot().diagnosis.summary_text; }
static std::wstring AreaText(const wchar_t* prefix, const AreaStatus& a) {
    if (a.status == CheckStatus::Bad) return std::wstring(prefix) + L" BAD";
    if (a.status == CheckStatus::Slow) return std::wstring(prefix) + L" SLOW";
    if (a.status == CheckStatus::Warning) return std::wstring(prefix) + L" WARN";
    if (a.avg_latency_ms >= 0) { std::wstringstream ss; ss << prefix << L" " << a.avg_latency_ms << L"ms"; return ss.str(); }
    return std::wstring(prefix) + L" ...";
}
std::wstring CDnsStatusItem::BuildValue() const { return AreaText(L"DNS", m_plugin.GetStateSnapshot().dns_status); }
std::wstring CCnStatusItem::BuildValue() const { return AreaText(L"CN", m_plugin.GetStateSnapshot().cn_status); }
std::wstring CIntlStatusItem::BuildValue() const { return AreaText(L"INTL", m_plugin.GetStateSnapshot().intl_status); }
std::wstring CProxyStatusItem::BuildValue() const {
    auto state = m_plugin.GetStateSnapshot();
    const auto& p = state.proxy_status;
    if (p.system_proxy_enabled || p.local_proxy_detected) return L"Proxy ON";
    return L"Proxy OFF";
}

std::wstring CPublicIpItem::BuildValue() const {
    auto state = m_plugin.GetStateSnapshot();
    const auto& ip = state.public_ip;
    if (!ip.enabled) return L"IP OFF";
    if (!ip.success) return L"IP BAD";
    if (ip.ip.empty()) return L"IP ?";
    return L"IP " + ip.ip;
}

std::wstring CDeveloperSitesItem::BuildValue() const {
    auto state = m_plugin.GetStateSnapshot();
    const auto& a = state.dev_status;
    if (a.total_count == 0) return L"DEV OFF";
    if (a.status == CheckStatus::Bad) return L"DEV BAD";
    if (a.status == CheckStatus::Slow) return L"DEV SLOW";
    if (a.success_count < a.total_count) {
        std::wstringstream ss; ss << L"DEV " << a.success_count << L"/" << a.total_count; return ss.str();
    }
    return L"DEV OK";
}

std::wstring CCustomSitesItem::BuildValue() const {
    auto state = m_plugin.GetStateSnapshot();
    const auto& a = state.custom_status;
    if (a.total_count == 0) return L"SITE OFF";
    if (a.status == CheckStatus::Bad) return L"SITE BAD";
    if (a.status == CheckStatus::Slow) return L"SITE SLOW";
    if (a.success_count < a.total_count) {
        std::wstringstream ss; ss << L"SITE " << a.success_count << L"/" << a.total_count; return ss.str();
    }
    return L"SITE OK";
}
