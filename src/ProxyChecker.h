#pragma once
#include "NetDoctorTypes.h"
#include <vector>

class ProxyChecker {
public:
    ProxyStatus Check(const std::vector<EndpointConfig>& ports, bool detect_system_proxy, int timeout_ms);
private:
    bool IsTcpOpen(const std::wstring& host, unsigned short port, int timeout_ms);
};
