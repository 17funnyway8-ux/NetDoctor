#pragma once
#include "NetDoctorTypes.h"
#include <string>
#include <vector>

class DnsChecker {
public:
    std::vector<DnsCheckResult> Check(const std::vector<std::wstring>& domains);
};
