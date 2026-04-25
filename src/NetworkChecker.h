#pragma once
#include "ConfigManager.h"
#include "DnsChecker.h"
#include "HttpChecker.h"
#include "ProxyChecker.h"
#include "PublicIpChecker.h"

class NetworkChecker {
public:
    NetDoctorState CheckAll(const ConfigManager& config);
private:
    DnsChecker m_dns;
    HttpChecker m_http;
    ProxyChecker m_proxy;
    PublicIpChecker m_public_ip;
};
