#pragma once
#include "NetDoctorTypes.h"
#include "ConfigManager.h"

class DiagnosisEngine {
public:
    void Analyze(NetDoctorState& state, const ConfigManager& config);
private:
    AreaStatus AnalyzeHttp(const std::vector<HttpCheckResult>& results, int good_ms, int slow_ms);
    AreaStatus AnalyzeDns(const std::vector<DnsCheckResult>& results, int good_ms, int slow_ms);
};
