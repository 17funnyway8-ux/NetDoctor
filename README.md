# NetDoctor

NetDoctor 是一个 TrafficMonitor 网络质量诊断插件 MVP。当前已加入后台线程异步检测，避免阻塞 TrafficMonitor UI。它不重复显示网速，而是补充诊断：DNS、国内/国际连通性、代理状态，并在 Tooltip 里展示详细检查结果。

## MVP 显示项

- 网络诊断：`NET OK` / `DNS BAD` / `INTL BAD` / `INTL SLOW`
- DNS 状态：`DNS 23ms` / `DNS SLOW` / `DNS BAD`
- 国内网络：`CN 32ms` / `CN BAD`
- 国际网络：`INTL 220ms` / `INTL BAD`
- 代理状态：`Proxy ON` / `Proxy OFF`
- 公网出口：`IP OFF` / `IP 1.2.3.4` / `IP BAD`
- 开发者站点：`DEV OFF` / `DEV OK` / `DEV 4/5` / `DEV BAD`
- 自定义站点：`SITE OFF` / `SITE OK` / `SITE 3/4` / `SITE BAD`
- Ping 状态：`PING OFF` / `PING 32ms` / `PING SLOW` / `PING BAD`
- AI 服务：`AI OFF` / `AI OK` / `AI 4/6` / `AI BAD`
- 家庭网络：`LAN OFF` / `LAN OK` / `LAN 3/4` / `LAN BAD`

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

[Developer]
Enabled=0
Targets=GitHub|https://github.com,GitHubRaw|https://raw.githubusercontent.com,npm|https://registry.npmjs.org,PyPI|https://pypi.org,Docker|https://hub.docker.com

[Proxy]
Enabled=1
DetectSystemProxy=1
Ports=127.0.0.1:7890,127.0.0.1:7897,127.0.0.1:1080,127.0.0.1:20171

[AI]
Enabled=0
Targets=OpenAI|https://api.openai.com,Anthropic|https://api.anthropic.com,Gemini|https://generativelanguage.googleapis.com,HuggingFace|https://huggingface.co,Ollama|http://127.0.0.1:11434,LMStudio|http://127.0.0.1:1234

[Home]
Enabled=0
Targets=Router|http://192.168.1.1,NAS|http://192.168.1.10:5000,HomeAssistant|http://homeassistant.local:8123,Jellyfin|http://127.0.0.1:8096

[Ping]
Enabled=0
Hosts=223.5.5.5,1.1.1.1,github.com
Count=4

[CustomSites]
Enabled=0
Targets=NAS|http://192.168.1.10:5000,Router|http://192.168.1.1,Blog|https://example.com

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
- V0.4：开发者站点 Profile，默认关闭，可检测 GitHub、GitHub Raw、npm、PyPI、Docker Hub。
- V0.5：自定义站点监控，默认关闭，可检测 NAS、路由器、自建 API、博客等 HTTP/HTTPS 目标。
- V0.6：Ping / 丢包率检测，默认关闭，使用 Windows ICMP API。
- V0.7：AI Profile，默认关闭，检测 OpenAI、Anthropic、Gemini、Hugging Face、Ollama、LM Studio。
- V0.8：Home/LAN Profile，默认关闭，检测路由器、NAS、Home Assistant、Jellyfin 等。
- V1.0：提供基础配置入口；插件管理里点击“选项”会打开 `NetDoctor.ini`。
- V1.1：提供原生 Win32 图形化配置窗口；点击“选项”可编辑主要 INI 配置，保存后即时 reload 并触发后台重新检测。

## 当前限制

- 虽然已异步检测，但目标数量仍不宜过多。
- 图形化设置界面已覆盖主要 INI 项，但暂未做更复杂的目标列表表格编辑器。
- 暂未实现自绘状态指示、历史统计。
- HTTP 优先使用 HEAD，请求失败或遇到 403/405 时 fallback 到 GET。

## 设计文档

详见：`NetDoctor_产品设计与技术方案.md`

## 隐私说明

公网 IP 检测默认关闭。只有将 `[PublicIP] Enabled=1` 后，插件才会访问配置中的第三方 IP 查询接口。AI、开发者、自定义站点、Home/LAN 和 Ping Profile 也默认关闭，只有显式启用后才会访问配置中的目标。
