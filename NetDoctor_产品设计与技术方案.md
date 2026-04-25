# NetDoctor：TrafficMonitor 网络质量诊断插件产品设计与技术实现方案

## 1. 插件定位

### 插件名

推荐名称：**NetDoctor**

备选名称：

- NetPulse
- NetQuality
- NetLens
- NetDiag
- NetStatus

最推荐 **NetDoctor**，因为它表达的是“诊断网络问题”，不是单纯测速。

### 一句话定位

> NetDoctor 是一个用于 TrafficMonitor 的网络质量诊断插件，用于显示 DNS、国内/国际连通性、代理/VPN 状态、开发者站点可达性，并给出简短网络诊断结论。

### 和 TrafficMonitor 主程序的关系

TrafficMonitor 已经负责显示：

- 当前上传速度
- 当前下载速度
- CPU / 内存 / 硬件状态

NetDoctor 补充显示：

- 网络质量是否正常
- DNS 是否慢
- GitHub / 开发者网站是否可达
- 代理/VPN 是否工作
- 当前出口 IP 地区
- 是“全网断了”还是“国外慢了”还是“代理挂了”

核心差异：

> TrafficMonitor 显示“流量速度”，NetDoctor 解释“网络健康”。

---

## 2. 目标用户

### 开发者

他们经常关心：

- GitHub 是否能访问
- npm / PyPI / Docker Hub 是否正常
- 代理是否开启
- DNS 是否异常
- 本地服务是否可达

### 科学上网 / 多网络环境用户

他们关心：

- 当前是否走代理
- 出口 IP 在哪里
- 国内/国际访问是否分流正常
- VPN/TUN 是否失效

### 家庭服务器/NAS 用户

他们关心：

- NAS 是否在线
- 路由器是否在线
- Home Assistant 是否在线
- 内网服务是否挂了

### AI 用户

他们关心：

- OpenAI / Anthropic / Gemini / Hugging Face 是否可达
- Ollama / LM Studio 是否运行
- 代理是否影响 API 调用

---

## 3. 核心产品原则

### 3.1 不做另一个测速工具

不要重复 Speedtest 的功能。

重点不是：

```text
下载速度 300Mbps
上传速度 50Mbps
```

而是：

```text
DNS SLOW
GH TIMEOUT
PROXY DOWN
INTL BAD
```

### 3.2 显示要极简，详情放 Tooltip

任务栏空间很小，显示项必须短。

主显示：

```text
NET OK
DNS 23ms
GH 218ms
Proxy ON
```

Tooltip 展示细节：

```text
Diagnosis: 国际网络较慢，国内网络正常，代理可用。

CN HTTP: 38ms OK
GitHub: 218ms SLOW
DNS: 23ms OK
Proxy 127.0.0.1:7890: OPEN
Public IP: Japan
Last Check: 21:08:32
```

### 3.3 自动诊断比原始指标更重要

用户不想看一堆数字，他们想知道哪里坏了。

所以插件应该输出诊断结论：

```text
NET OK
DNS SLOW
INTL BAD
PROXY OFF
GH SLOW
LAN OK
```

---

## 4. 插件显示项设计

一个 DLL 插件可以提供多个 `IPluginItem`。建议 NetDoctor 第一版提供以下显示项。

### 4.1 Net Summary

名称：`网络诊断`

Item ID：`NetDoctorSummary`

显示示例：

```text
NET OK
INTL SLOW
PROXY BAD
DNS BAD
NET BAD
SITE BAD
```

作用：这是总览项目，适合绝大多数用户只显示这一个。

诊断逻辑：

| 条件 | 显示 |
|---|---|
| 国内、国际、DNS、代理均正常 | `NET OK` |
| 国内正常，国际慢或失败 | `INTL SLOW` / `INTL BAD` |
| DNS 解析慢或失败 | `DNS SLOW` / `DNS BAD` |
| 国内国际都失败 | `NET BAD` |
| 代理端口开放但代理请求失败 | `PROXY BAD` |
| 自定义站点部分失败 | `SITE BAD` |

### 4.2 DNS Status

名称：`DNS 状态`

Item ID：`NetDoctorDNS`

显示示例：

```text
DNS 23ms
DNS SLOW
DNS BAD
```

作用：显示 DNS 解析耗时和状态。

默认测试：

- `github.com`
- `www.baidu.com`
- `cloudflare.com`

判断标准：

| 状态 | 条件 |
|---|---|
| OK | 解析成功且平均耗时 < 100ms |
| SLOW | 解析成功但平均耗时 >= 100ms |
| BAD | 多数域名解析失败 |

### 4.3 CN Latency

名称：`国内网络`

Item ID：`NetDoctorCN`

显示示例：

```text
CN 32ms
CN SLOW
CN BAD
```

默认目标：

- `https://www.baidu.com`
- `https://www.qq.com`
- `https://www.aliyun.com`

作用：判断国内基础网络是否正常。

### 4.4 Intl Latency

名称：`国际网络`

Item ID：`NetDoctorIntl`

显示示例：

```text
INTL 220ms
INTL SLOW
INTL BAD
```

默认目标：

- `https://github.com`
- `https://www.cloudflare.com`

说明：考虑国内环境，`google.com` 可能默认不开启，避免一装上就全红。Google / OpenAI / Anthropic 可以后续放到 AI Profile 中，默认关闭。

### 4.5 Developer Sites

名称：`开发者站点`

Item ID：`NetDoctorDevSites`

显示示例：

```text
DEV OK
DEV 5/6
GH BAD
NPM BAD
```

默认目标：

- GitHub: `https://github.com`
- GitHub Raw: `https://raw.githubusercontent.com`
- npm: `https://registry.npmjs.org`
- PyPI: `https://pypi.org`
- Docker Hub: `https://hub.docker.com`

作用：面向开发者，判断常用开发资源是否可用。

### 4.6 Proxy Status

名称：`代理状态`

Item ID：`NetDoctorProxy`

显示示例：

```text
Proxy ON
Proxy OFF
Proxy BAD
VPN ON
```

检测内容：

第一版可以检测常见本地代理端口：

- `127.0.0.1:7890`
- `127.0.0.1:7897`
- `127.0.0.1:1080`
- `127.0.0.1:20171`

同时读取 Windows 系统代理设置：

- 是否开启系统 HTTP 代理
- 代理服务器地址
- 是否启用自动配置脚本

判断标准：

| 状态 | 条件 |
|---|---|
| Proxy ON | 系统代理开启或常见代理端口开放 |
| Proxy OFF | 没有检测到代理 |
| Proxy BAD | 代理端口开放，但代理请求失败 |
| VPN ON | 出口 IP 与本地网络特征明显不同，或检测到常见 VPN 网卡 |

### 4.7 Public IP

名称：`公网出口`

Item ID：`NetDoctorPublicIP`

显示示例：

```text
IP CN
IP JP
IP US
IP ?
```

数据源：

- `https://api.ipify.org`
- `https://ifconfig.me/ip`
- `https://ipinfo.io/json`
- `https://ipapi.co/json`

考虑稳定性，建议配置多个接口，失败时 fallback。

Tooltip 信息：

```text
Public IP: 1.2.3.4
Country: JP
Region: Tokyo
ISP: Example ISP
Changed: 21:03:12
```

注意：公网 IP 查询不要太频繁，默认 5-10 分钟一次即可。

### 4.8 Custom Sites

名称：`自定义站点`

Item ID：`NetDoctorCustomSites`

显示示例：

```text
SITE OK
SITE 4/5
NAS BAD
API BAD
```

用途：允许用户配置自己的目标：

- NAS
- 路由器
- 博客
- 自建 API
- Home Assistant
- 公司内网服务

---

## 5. Tooltip 设计

Tooltip 是这个插件的价值核心之一。

示例：

```text
NetDoctor

Diagnosis:
国际网络较慢，国内网络正常，代理已开启。

Summary:
CN: OK 34ms
INTL: SLOW 248ms
DNS: OK 21ms
Proxy: ON 127.0.0.1:7890
Public IP: JP / Tokyo

Developer:
GitHub: OK 220ms
GitHub Raw: SLOW 680ms
npm: OK 188ms
PyPI: OK 240ms
Docker Hub: TIMEOUT

Last Check:
2026-04-25 21:10:32
```

---

## 6. 配置文件设计

建议使用 INI 或 JSON。

TrafficMonitor 插件传统上 C++/Windows 环境比较自然的是 INI，但 JSON 对复杂配置更友好。

建议：

- MVP 使用 `NetDoctor.ini`
- 后续如果配置复杂，再支持 `NetDoctor.json`

推荐目录结构：

```text
plugins/
  NetDoctor.dll
  NetDoctor/
    NetDoctor.ini
    cache.json
    log.txt
```

### 6.1 MVP 配置文件示例

```ini
[General]
CheckIntervalSeconds=30
TimeoutMilliseconds=3000
EnableTooltip=1
Language=zh-CN

[Threshold]
LatencyGoodMs=100
LatencySlowMs=300
DnsGoodMs=80
DnsSlowMs=200
PacketLossWarningPercent=5

[DNS]
Enabled=1
Domains=www.baidu.com,github.com,cloudflare.com

[CN]
Enabled=1
Targets=https://www.baidu.com,https://www.qq.com,https://www.aliyun.com

[International]
Enabled=1
Targets=https://github.com,https://www.cloudflare.com

[Developer]
Enabled=1
Targets=GitHub|https://github.com,GitHubRaw|https://raw.githubusercontent.com,npm|https://registry.npmjs.org,PyPI|https://pypi.org,Docker|https://hub.docker.com

[Proxy]
Enabled=1
DetectSystemProxy=1
Ports=127.0.0.1:7890,127.0.0.1:7897,127.0.0.1:1080,127.0.0.1:20171

[PublicIP]
Enabled=1
CheckIntervalSeconds=600
Providers=https://api.ipify.org,https://ifconfig.me/ip

[CustomSites]
Enabled=0
Targets=NAS|http://192.168.1.10:5000,Router|http://192.168.1.1,Blog|https://example.com
```

### 6.2 后续 JSON 配置示例

```json
{
  "general": {
    "checkIntervalSeconds": 30,
    "timeoutMilliseconds": 3000,
    "language": "zh-CN"
  },
  "threshold": {
    "latencyGoodMs": 100,
    "latencySlowMs": 300,
    "dnsGoodMs": 80,
    "dnsSlowMs": 200
  },
  "profiles": {
    "basic": true,
    "developer": true,
    "ai": false,
    "home": false
  },
  "targets": {
    "cn": [
      { "name": "Baidu", "url": "https://www.baidu.com" },
      { "name": "QQ", "url": "https://www.qq.com" }
    ],
    "international": [
      { "name": "GitHub", "url": "https://github.com" },
      { "name": "Cloudflare", "url": "https://www.cloudflare.com" }
    ],
    "developer": [
      { "name": "npm", "url": "https://registry.npmjs.org" },
      { "name": "PyPI", "url": "https://pypi.org" }
    ]
  }
}
```

---

## 7. 数据采集方式

### 7.1 HTTP 可达性检测

推荐使用 **WinHTTP**。

原因：

- Windows 原生
- 不引入额外 DLL
- 适合后台 HTTP 检测
- 可控超时

检测指标：

```cpp
struct HttpCheckResult
{
    std::wstring name;
    std::wstring url;
    bool success;
    int status_code;
    int latency_ms;
    std::wstring error_message;
};
```

判断：

- HTTP 2xx / 3xx：成功
- HTTP 4xx：可访问但业务异常，可标记为 `WARN`
- HTTP 5xx：目标服务异常
- timeout / DNS fail / connect fail：失败

### 7.2 DNS 检测

MVP 可以用 `GetAddrInfoW`，简单够用。

检测流程：

1. 记录开始时间
2. 调用 `GetAddrInfoW(domain)`
3. 记录结束时间
4. 计算耗时
5. 保存 IP 列表数量

结果结构：

```cpp
struct DnsCheckResult
{
    std::wstring domain;
    bool success;
    int latency_ms;
    std::vector<std::wstring> addresses;
    std::wstring error_message;
};
```

### 7.3 Ping 延迟 / 丢包率

后续版本使用 Windows ICMP API：

- `IcmpCreateFile`
- `IcmpSendEcho`
- `IcmpCloseHandle`

注意：Ping 可能被目标站禁用，所以它不能作为唯一判断标准。

MVP 可以先不做 Ping，只做 HTTP 延迟和 DNS 延迟。

后续结构：

```cpp
struct PingResult
{
    std::wstring host;
    bool success;
    int sent;
    int received;
    int loss_percent;
    int avg_latency_ms;
    int min_latency_ms;
    int max_latency_ms;
};
```

### 7.4 系统代理检测

读取 Windows Internet Settings 注册表：

```text
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings
```

字段：

```text
ProxyEnable
ProxyServer
AutoConfigURL
```

也可以使用 WinHTTP API：

- `WinHttpGetIEProxyConfigForCurrentUser`

结果结构：

```cpp
struct ProxyStatus
{
    bool system_proxy_enabled;
    std::wstring proxy_server;
    bool auto_config_enabled;
    std::wstring auto_config_url;

    bool local_proxy_detected;
    std::wstring detected_proxy_address;

    bool proxy_usable;
};
```

### 7.5 本地代理端口检测

方法：普通 TCP connect。

检测：

```text
127.0.0.1:7890
127.0.0.1:7897
127.0.0.1:1080
127.0.0.1:20171
```

判断：

- 能 connect：端口开放
- 不能 connect：端口关闭

注意：端口开放不代表代理一定可用，所以后续可以增加“通过代理访问目标 URL”的检测。

### 7.6 公网 IP 检测

方法：HTTP GET。

IP：

- `https://api.ipify.org`
- `https://ifconfig.me/ip`

地区信息：

- `https://ipinfo.io/json`
- `https://ipapi.co/json`

频率：公网 IP 检测不应频繁，推荐 600 秒一次。

结果结构：

```cpp
struct PublicIpInfo
{
    bool success;
    std::wstring ip;
    std::wstring country;
    std::wstring region;
    std::wstring city;
    std::wstring isp;
    std::wstring error_message;
    std::chrono::system_clock::time_point last_update;
};
```

---

## 8. 总体诊断逻辑

### 8.1 基础状态枚举

```cpp
enum class CheckStatus
{
    Unknown,
    Ok,
    Slow,
    Warning,
    Bad
};
```

### 8.2 网络区域状态

```cpp
struct AreaStatus
{
    CheckStatus status;
    int avg_latency_ms;
    int success_count;
    int total_count;
};
```

### 8.3 总体诊断结果

```cpp
struct DiagnosisResult
{
    CheckStatus overall_status;
    std::wstring summary_text;
    std::wstring detail_text;
};
```

### 8.4 判断优先级

建议优先级：

1. 全部失败：`NET BAD`
2. DNS 失败：`DNS BAD`
3. 国内失败、国际失败：`NET BAD`
4. 国内正常、国际失败：`INTL BAD`
5. 国内正常、国际慢：`INTL SLOW`
6. 代理开启但不可用：`PROXY BAD`
7. 自定义站点失败：`SITE BAD`
8. 全部正常：`NET OK`

示例伪代码：

```cpp
if (dns.status == Bad)
    return L"DNS BAD";

if (cn.status == Bad && intl.status == Bad)
    return L"NET BAD";

if (cn.status == Ok && intl.status == Bad)
    return L"INTL BAD";

if (cn.status == Ok && intl.status == Slow)
    return L"INTL SLOW";

if (proxy.enabled && !proxy.proxy_usable)
    return L"PROXY BAD";

if (dev.failed_count > 0)
    return L"DEV BAD";

return L"NET OK";
```

---

## 9. C++ 类结构设计

### 9.1 文件结构

```text
NetDoctor/
  include/
    PluginInterface.h

  src/
    NetDoctorPlugin.h
    NetDoctorPlugin.cpp

    NetDoctorItems.h
    NetDoctorItems.cpp

    ConfigManager.h
    ConfigManager.cpp

    NetworkChecker.h
    NetworkChecker.cpp

    DnsChecker.h
    DnsChecker.cpp

    HttpChecker.h
    HttpChecker.cpp

    ProxyChecker.h
    ProxyChecker.cpp

    PublicIpChecker.h
    PublicIpChecker.cpp

    DiagnosisEngine.h
    DiagnosisEngine.cpp

    Utils.h
    Utils.cpp

  resources/
    NetDoctor.rc

  NetDoctor.sln
  NetDoctor.vcxproj
  README.md
```

### 9.2 插件主类：`CNetDoctorPlugin`

继承：

```cpp
class CNetDoctorPlugin : public ITMPlugin
```

职责：

- 管理所有显示项
- 加载配置
- 定时触发数据采集
- 保存最新诊断结果
- 提供插件信息
- 提供 Tooltip

核心方法：

```cpp
class CNetDoctorPlugin : public ITMPlugin
{
public:
    static CNetDoctorPlugin& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual void DataRequired() override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;

    const NetDoctorState& GetState() const;

private:
    CNetDoctorPlugin();

    void LoadConfig();
    void UpdateDataIfNeeded();

private:
    std::vector<std::unique_ptr<IPluginItem>> m_items;
    ConfigManager m_config;
    NetworkChecker m_checker;
    DiagnosisEngine m_diagnosis;

    NetDoctorState m_state;
    std::wstring m_tooltip_text;

    std::chrono::steady_clock::time_point m_last_check_time;
};
```

### 9.3 插件显示项基类：`CNetDoctorItemBase`

```cpp
class CNetDoctorItemBase : public IPluginItem
{
public:
    CNetDoctorItemBase(CNetDoctorPlugin& plugin);

    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;

protected:
    CNetDoctorPlugin& m_plugin;

    std::wstring m_name;
    std::wstring m_id;
    std::wstring m_label;
    mutable std::wstring m_value;
    std::wstring m_sample;
};
```

### 9.4 各显示项类

```cpp
class CNetSummaryItem : public CNetDoctorItemBase
{
public:
    const wchar_t* GetItemValueText() const override;
};

class CDnsStatusItem : public CNetDoctorItemBase
{
public:
    const wchar_t* GetItemValueText() const override;
};

class CCnLatencyItem : public CNetDoctorItemBase
{
public:
    const wchar_t* GetItemValueText() const override;
};

class CIntlLatencyItem : public CNetDoctorItemBase
{
public:
    const wchar_t* GetItemValueText() const override;
};

class CProxyStatusItem : public CNetDoctorItemBase
{
public:
    const wchar_t* GetItemValueText() const override;
};

class CPublicIpItem : public CNetDoctorItemBase
{
public:
    const wchar_t* GetItemValueText() const override;
};

class CDevSitesItem : public CNetDoctorItemBase
{
public:
    const wchar_t* GetItemValueText() const override;
};
```

### 9.5 状态结构：`NetDoctorState`

```cpp
struct NetDoctorState
{
    DiagnosisResult diagnosis;

    AreaStatus cn_status;
    AreaStatus intl_status;
    AreaStatus dev_status;

    std::vector<DnsCheckResult> dns_results;
    std::vector<HttpCheckResult> cn_results;
    std::vector<HttpCheckResult> intl_results;
    std::vector<HttpCheckResult> dev_results;
    std::vector<HttpCheckResult> custom_results;

    ProxyStatus proxy_status;
    PublicIpInfo public_ip;

    std::wstring last_error;
    std::chrono::system_clock::time_point last_update;
};
```

### 9.6 配置管理：`ConfigManager`

```cpp
class ConfigManager
{
public:
    bool Load(const std::wstring& path);
    bool SaveDefaultIfNotExists(const std::wstring& path);

    int GetCheckIntervalSeconds() const;
    int GetTimeoutMilliseconds() const;

    std::vector<std::wstring> GetDnsDomains() const;
    std::vector<TargetConfig> GetCnTargets() const;
    std::vector<TargetConfig> GetIntlTargets() const;
    std::vector<TargetConfig> GetDeveloperTargets() const;
    std::vector<TargetConfig> GetCustomTargets() const;

    std::vector<EndpointConfig> GetProxyPorts() const;

private:
    // MVP 可先用 Windows GetPrivateProfileStringW 读取 INI
};
```

目标配置：

```cpp
struct TargetConfig
{
    std::wstring name;
    std::wstring url;
    bool enabled;
};
```

### 9.7 网络检测聚合：`NetworkChecker`

```cpp
class NetworkChecker
{
public:
    NetworkChecker();

    NetDoctorState CheckAll(const ConfigManager& config);

private:
    DnsChecker m_dns_checker;
    HttpChecker m_http_checker;
    ProxyChecker m_proxy_checker;
    PublicIpChecker m_public_ip_checker;
};
```

### 9.8 诊断引擎：`DiagnosisEngine`

```cpp
class DiagnosisEngine
{
public:
    DiagnosisResult Diagnose(const NetDoctorState& state);

private:
    AreaStatus CalculateAreaStatus(const std::vector<HttpCheckResult>& results);
    CheckStatus EvaluateLatency(int latency_ms, int good_ms, int slow_ms);
};
```

---

## 10. 多线程与性能设计

TrafficMonitor 会定期调用：

```cpp
ITMPlugin::DataRequired()
```

注意事项：

- 不要在 UI 线程里做长时间阻塞网络请求。
- `GetItemValueText()` 会频繁调用，不能在里面请求网络。
- 网络请求应当在 `DataRequired()` 中触发。
- 但 `DataRequired()` 也不应长时间阻塞。

### MVP 简化方案

第一版可以先做同步检测，但必须：

- 总超时时间短
- 每个请求 2-3 秒超时
- 每轮检测间隔 30 秒
- 目标数量少

MVP 默认目标控制在：

- DNS 3 个
- CN 2 个
- INTL 2 个
- Proxy 4 个端口

这样压力可控。

### 更好的方案

V0.2 开始改为后台线程：

```cpp
std::thread m_worker_thread;
std::mutex m_state_mutex;
std::atomic<bool> m_checking;
```

流程：

1. `DataRequired()` 判断是否到检测时间。
2. 如果没有检测线程在跑，则启动后台检测。
3. 后台线程完成后更新 `m_state`。
4. 显示项只读取最新状态。

这样 UI 不会卡。

---

## 11. MVP 开发步骤

### 阶段 0：准备工程

目标：

- 能编译出 DLL
- 能被 TrafficMonitor 加载
- 插件管理里能看到插件

任务：

1. 获取 `PluginInterface.h`
2. 创建 Visual Studio C++ DLL 项目
3. 实现 `TMPluginGetInstance`
4. 实现 `CNetDoctorPlugin`
5. 实现一个最简单的显示项：

```text
NET OK
```

验收标准：

- DLL 放入 `plugins` 后 TrafficMonitor 能加载。
- 插件管理窗口能显示 NetDoctor。
- 显示项目中能看到“网络诊断”。

### 阶段 1：实现配置文件

目标：

- 自动创建默认配置
- 读取检测间隔、超时、目标列表

任务：

1. 实现 `ConfigManager`
2. 如果 `plugins/NetDoctor/NetDoctor.ini` 不存在，则创建默认配置
3. 支持读取：
   - General
   - DNS
   - CN
   - International
   - Proxy

验收标准：

- 修改 INI 后重启 TrafficMonitor 生效。
- 目标 URL 可配置。

### 阶段 2：实现 HTTP 检测

目标：

- 检测国内/国际网站可达性和耗时

任务：

1. 实现 `HttpChecker`
2. 使用 WinHTTP 发起 GET 或 HEAD 请求
3. 记录：
   - success
   - status code
   - latency
   - error
4. 实现 CN / INTL 两个状态项：

```text
CN 32ms
INTL 220ms
```

验收标准：

- 断网时显示 `CN BAD` / `INTL BAD`
- 网络慢时显示 `SLOW`
- Tooltip 能看到每个目标的结果

### 阶段 3：实现 DNS 检测

目标：

- 检测 DNS 是否慢或失败

任务：

1. 实现 `DnsChecker`
2. 使用 `GetAddrInfoW`
3. 统计平均解析耗时
4. 实现 DNS 显示项：

```text
DNS 23ms
```

验收标准：

- 正常网络显示 DNS 耗时
- DNS 异常时显示 `DNS BAD`
- Tooltip 展示每个域名解析结果

### 阶段 4：实现代理检测

目标：

- 检测系统代理和常见代理端口

任务：

1. 实现 `ProxyChecker`
2. 读取系统代理配置
3. TCP connect 检测本地端口
4. 实现 Proxy 显示项：

```text
Proxy ON
Proxy OFF
```

验收标准：

- Clash / v2rayN / sing-box 等运行时能显示 `Proxy ON`
- 关闭代理后显示 `Proxy OFF`
- Tooltip 显示检测到的代理端口

### 阶段 5：实现总诊断

目标：

- 聚合所有结果，输出简短结论

任务：

1. 实现 `DiagnosisEngine`
2. 根据 DNS / CN / INTL / Proxy 生成 summary
3. 实现 Summary 显示项：

```text
NET OK
INTL BAD
DNS SLOW
PROXY BAD
```

验收标准：

- 国内正常、GitHub 超时时显示 `INTL BAD`
- DNS 失败时优先显示 `DNS BAD`
- 全部正常时显示 `NET OK`

### 阶段 6：实现 Tooltip

目标：

- 鼠标悬停显示详细诊断信息

任务：

1. 在 `GetTooltipInfo()` 返回格式化文本
2. 包含：
   - Diagnosis
   - CN 结果
   - INTL 结果
   - DNS 结果
   - Proxy 状态
   - Last Check

验收标准：

- Tooltip 信息清晰
- 不返回空指针
- 不在 Tooltip 阶段做网络请求

### 阶段 7：打包发布 MVP

目标：用户可下载使用。

打包结构：

```text
NetDoctor_v0.1_x64.zip
  NetDoctor.dll
  NetDoctor/
    NetDoctor.ini
  README.md
```

README 包含：

- 安装方法
- 配置方法
- 显示项说明
- 常见问题
- 已知限制

---

## 12. MVP 功能范围

### V0.1 必做

- TrafficMonitor 插件加载
- Summary 显示项
- DNS 显示项
- CN 显示项
- INTL 显示项
- Proxy 显示项
- INI 配置
- Tooltip 详情
- WinHTTP HTTP 检测
- GetAddrInfoW DNS 检测
- 常见本地代理端口检测

### V0.1 不做

- 图形化设置界面
- 丢包率
- 路由追踪
- 公网 IP 地区
- 历史图表
- 通知提醒
- 自绘 UI
- AI profile
- Docker / WSL 检测

这样可以避免第一版过重。

---

## 13. 后续版本路线图

### V0.2：稳定性与异步检测

目标：

- 不阻塞 TrafficMonitor
- 更稳定

功能：

- 后台线程检测
- 检测中状态：`CHECK...`
- 请求超时控制
- 检测失败重试
- 日志文件
- 更好的错误信息

### V0.3：公网 IP / 出口地区

功能：

- 查询公网 IP
- 显示国家/地区：`IP CN` / `IP JP` / `IP US`
- IP 变化记录
- Tooltip 显示 ISP / 地区

### V0.4：开发者 Profile

功能：

- GitHub
- GitHub Raw
- npm
- PyPI
- Docker Hub
- Maven Central
- NuGet
- crates.io

显示：

```text
DEV OK
DEV 5/7
GH BAD
NPM SLOW
```

### V0.5：自定义站点监控

功能：

- 用户自定义目标站点
- 支持 HTTP/HTTPS
- 支持 TCP 端口
- 支持局域网服务

示例：

```ini
[CustomSites]
Targets=NAS|http://192.168.1.10:5000,Router|http://192.168.1.1,API|https://api.example.com
```

显示：

```text
SITE 3/4
NAS BAD
```

### V0.6：Ping / 丢包率

功能：

- ICMP Ping
- 平均延迟
- 丢包率
- 最大/最小延迟

显示：

```text
PING 32ms
LOSS 5%
```

注意：Ping 不是所有目标都支持，所以它应该是补充检测，不是主判断依据。

### V0.7：AI Profile

功能：

- OpenAI API
- Anthropic API
- Gemini API
- Hugging Face
- Ollama `localhost:11434`
- LM Studio `localhost:1234`

显示：

```text
AI OK
AI 3/5
Ollama ON
LLM BAD
```

### V0.8：Home/LAN Profile

功能：

- 路由器
- NAS
- Home Assistant
- Jellyfin
- Plex
- 摄像头/NVR
- 自定义内网服务

显示：

```text
LAN OK
NAS BAD
HA OK
```

### V1.0：图形化配置界面

功能：

- `ShowOptionsDialog`
- 可视化编辑目标
- 阈值设置
- Profile 开关
- 代理端口配置
- 检测间隔配置

这一步才值得做 GUI。太早做 GUI 会拖慢核心功能开发。

### V1.1：自绘状态指示

功能：

- 彩色状态点
- 简单柱状延迟图
- 深色模式适配
- 高 DPI 适配

显示效果：

```text
● NET OK
● GH SLOW
● DNS BAD
```

这个需要实现：

```cpp
IsCustomDraw()
DrawItem()
GetItemWidthEx()
```

### V1.2：历史质量统计

功能：

- 最近 1 小时 / 24 小时网络质量
- 最近失败次数
- 最差目标
- 网络抖动记录
- IP 变化记录

但这个版本要注意不要写太多磁盘日志。

---

## 14. 风险与注意事项

### 14.1 不要频繁请求外部服务

避免插件变成“流量制造机”。

默认建议：

```text
普通检测：30 秒一次
公网 IP：10 分钟一次
开发者站点：60 秒一次
```

### 14.2 不能在 `GetItemValueText()` 里请求网络

这个函数可能频繁调用，只能返回缓存字符串。

正确方式：

- `DataRequired()` 触发检测
- 检测结果保存到状态对象
- `GetItemValueText()` 读取状态对象

### 14.3 网络请求必须有超时

否则 TrafficMonitor 可能被拖慢。

推荐：

```text
HTTP Timeout: 3000ms
DNS Timeout: 系统限制，尽量减少域名数量
TCP Connect Timeout: 1000ms
```

### 14.4 不要默认检测太多国外站点

国内环境下，默认检测太多不可达网站，会导致用户误以为插件坏了。

MVP 国际默认目标建议：

- GitHub
- Cloudflare

Google / OpenAI / Anthropic 可以放到 AI profile 里，默认关闭。

### 14.5 隐私原则

插件不要上传用户数据。

公网 IP 查询不可避免会访问第三方接口，需要在 README 里说明。

配置里可以允许关闭：

```ini
[PublicIP]
Enabled=0
```

---

## 15. 推荐 MVP 显示效果

用户第一次装上后，任务栏可以显示：

```text
NET OK
DNS 21ms
CN 34ms
GH 226ms
Proxy ON
```

如果网络异常：

```text
INTL BAD
DNS 180ms
CN OK
GH BAD
Proxy OFF
```

Tooltip：

```text
NetDoctor

Diagnosis:
国际网络异常，国内网络正常，可能是代理/VPN 未开启或国际出口异常。

CN:
Baidu: OK 32ms
QQ: OK 41ms

International:
GitHub: TIMEOUT
Cloudflare: OK 280ms

DNS:
www.baidu.com: OK 18ms
github.com: OK 43ms
cloudflare.com: OK 22ms

Proxy:
System Proxy: OFF
127.0.0.1:7890: CLOSED
127.0.0.1:1080: CLOSED

Last Check:
2026-04-25 21:12:43
```

---

## 16. 最终建议

第一阶段就按这个范围开工：

### 插件名

**NetDoctor**

### MVP 显示项

- `网络诊断`
- `DNS 状态`
- `国内网络`
- `国际网络`
- `代理状态`

### MVP 检测能力

- DNS 解析耗时
- 国内 HTTP 可达性
- GitHub / Cloudflare 可达性
- 常见本地代理端口
- Windows 系统代理
- Tooltip 诊断详情

### 第一版不碰

- GUI 设置
- 公网 IP
- AI profile
- Ping 丢包
- 历史统计
- 自绘图形

这样第一版能比较快做出来，而且已经有明显价值。后续再从 “NetDoctor 网络诊断” 扩展成 “开发者/AI/家庭网络状态中心”。
