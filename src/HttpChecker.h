#pragma once
#include "NetDoctorTypes.h"
#include <vector>

class HttpChecker {
public:
    std::vector<HttpCheckResult> Check(const std::vector<TargetConfig>& targets, int timeout_ms);
    HttpCheckResult CheckOne(const TargetConfig& target, int timeout_ms);
};
