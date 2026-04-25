#pragma once
#include "NetDoctorTypes.h"
#include <string>
#include <vector>

class PublicIpChecker {
public:
    PublicIpInfo Check(const std::vector<std::wstring>& providers, int timeout_ms);
private:
    bool FetchText(const std::wstring& url, int timeout_ms, std::wstring& body, std::wstring& error_message);
};
