#pragma once
#include "NetDoctorTypes.h"
#include <string>
#include <vector>

class PingChecker {
public:
    std::vector<PingResult> Check(const std::vector<std::wstring>& hosts, int count, int timeout_ms);
private:
    PingResult CheckOne(const std::wstring& host, int count, int timeout_ms);
    bool ResolveIpv4(const std::wstring& host, unsigned long& ip_addr, std::wstring& error_message);
};
