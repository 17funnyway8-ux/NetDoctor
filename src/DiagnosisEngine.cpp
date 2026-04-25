#include "DiagnosisEngine.h"

AreaStatus DiagnosisEngine::AnalyzeHttp(const std::vector<HttpCheckResult>& results, int good_ms, int slow_ms) {
    AreaStatus s; s.total_count = (int)results.size(); int total_ms = 0;
    for (auto& r : results) if (r.success) { s.success_count++; total_ms += r.latency_ms; }
    if (s.total_count == 0) { s.status = CheckStatus::Unknown; return s; }
    if (s.success_count == 0) { s.status = CheckStatus::Bad; return s; }
    s.avg_latency_ms = total_ms / s.success_count;
    if (s.success_count < s.total_count) s.status = CheckStatus::Warning;
    else if (s.avg_latency_ms >= slow_ms) s.status = CheckStatus::Slow;
    else s.status = CheckStatus::Ok;
    return s;
}
AreaStatus DiagnosisEngine::AnalyzeDns(const std::vector<DnsCheckResult>& results, int good_ms, int slow_ms) {
    AreaStatus s; s.total_count = (int)results.size(); int total_ms = 0;
    for (auto& r : results) if (r.success) { s.success_count++; total_ms += r.latency_ms; }
    if (s.total_count == 0) { s.status = CheckStatus::Unknown; return s; }
    if (s.success_count == 0 || s.success_count * 2 < s.total_count) { s.status = CheckStatus::Bad; return s; }
    s.avg_latency_ms = total_ms / s.success_count;
    if (s.avg_latency_ms >= slow_ms) s.status = CheckStatus::Slow;
    else s.status = CheckStatus::Ok;
    return s;
}
void DiagnosisEngine::Analyze(NetDoctorState& state, const ConfigManager& config) {
    auto th = config.Thresholds();
    state.dns_status = AnalyzeDns(state.dns_results, th.dns_good_ms, th.dns_slow_ms);
    state.cn_status = AnalyzeHttp(state.cn_results, th.latency_good_ms, th.latency_slow_ms);
    state.intl_status = AnalyzeHttp(state.intl_results, th.latency_good_ms, th.latency_slow_ms);
    auto& d = state.diagnosis;
    d.overall_status = CheckStatus::Ok; d.summary_text = L"NET OK";
    if (state.dns_status.status == CheckStatus::Bad) { d.overall_status = CheckStatus::Bad; d.summary_text = L"DNS BAD"; }
    else if (state.cn_status.status == CheckStatus::Bad && state.intl_status.status == CheckStatus::Bad) { d.overall_status = CheckStatus::Bad; d.summary_text = L"NET BAD"; }
    else if (state.cn_status.status != CheckStatus::Bad && state.intl_status.status == CheckStatus::Bad) { d.overall_status = CheckStatus::Bad; d.summary_text = L"INTL BAD"; }
    else if (state.cn_status.status != CheckStatus::Bad && state.intl_status.status == CheckStatus::Slow) { d.overall_status = CheckStatus::Slow; d.summary_text = L"INTL SLOW"; }
    else if (state.dns_status.status == CheckStatus::Slow) { d.overall_status = CheckStatus::Slow; d.summary_text = L"DNS SLOW"; }
    else if (state.cn_status.status == CheckStatus::Slow) { d.overall_status = CheckStatus::Slow; d.summary_text = L"CN SLOW"; }
}
