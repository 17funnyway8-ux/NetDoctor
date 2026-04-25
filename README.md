# NetDoctor

NetDoctor 是一个 TrafficMonitor 网络质量诊断插件 MVP。当前已加入后台线程异步检测，避免阻塞 TrafficMonitor UI。它不重复显示网速，而是补充诊断：DNS、国内/国际连通性、代理状态，并在 Tooltip 里展示详细检查结果。

## MVP 显示项

- 网络诊断：`NET OK` / `DNS BAD` / `INTL BAD` / `INTL SLOW`
- DNS 状态：`DNS 23ms` / `DNS SLOW` / `DNS BAD`
- 国内网络：`CN 32ms` / `CN BAD`
- 国际网络：`INTL 220ms` / `INTL BAD`
- 代理状态：`Proxy ON` / `Proxy OFF`
- 公网出口：`IP OFF` / `IP 1.2.3.4` / `IP BAD`

## 构建

使用 Visual Studio 2022 打开 `NetDoctor.sln`，选择 `Release|x64` 或 `Release|Win32` 构建。

依赖均为 Windows SDK / 系统库：

- WinHTTP：HTTP HEAD 探测
- Ws2_32：DNS、TCP 端口探测

## 安装

将生成的 `NetDoctor.dll` 放入：

```text
TrafficMonitor.exe 所在目录/plugins/
```

重启 TrafficMonitor 后，在“插件管理”和“显示设置”中启用相关显示项。

## 配置

插件会自动创建：

```text
plugins/NetDoctor/NetDoctor.ini
```

默认配置：

```ini
[General]
CheckIntervalSeconds=30
TimeoutMilliseconds=3000

[Threshold]
LatencyGoodMs=100
LatencySlowMs=300
DnsGoodMs=80
DnsSlowMs=200

[DNS]
Domains=www.baidu.com,github.com,cloudflare.com

[CN]
Targets=Baidu|https://www.baidu.com,QQ|https://www.qq.com

[International]
Targets=GitHub|https://github.com,Cloudflare|https://www.cloudflare.com

[Proxy]
Enabled=1
DetectSystemProxy=1
Ports=127.0.0.1:7890,127.0.0.1:7897,127.0.0.1:1080,127.0.0.1:20171

[PublicIP]
Enabled=0
CheckIntervalSeconds=600
Providers=https://api.ipify.org,https://ifconfig.me/ip
```

## 当前进展

- V0.1：插件骨架、五个显示项、INI 配置、DNS/HTTP/代理检测、Tooltip。
- V0.2：后台线程异步检测；HTTP HEAD 失败后自动 fallback 到 GET，减少误报。
- 自查修复：避免后台线程 detach 后 DLL 卸载导致悬空访问；Tooltip 返回线程本地快照，避免返回锁内可变字符串指针。
- V0.3：公网 IP / 出口检测，默认关闭，需要用户在配置中启用。

## 当前限制

- 虽然已异步检测，但目标数量仍不宜过多。
- 暂无图形化设置界面。
- 暂未实现 Ping 丢包、开发者 Profile、AI Profile、历史统计。
- HTTP 优先使用 HEAD，请求失败或遇到 403/405 时 fallback 到 GET。

## 设计文档

详见：`NetDoctor_产品设计与技术方案.md`

## 隐私说明

公网 IP 检测默认关闭。只有将 `[PublicIP] Enabled=1` 后，插件才会访问配置中的第三方 IP 查询接口。
