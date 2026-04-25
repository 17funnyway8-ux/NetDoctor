#include "HttpChecker.h"
#include <windows.h>
#include <winhttp.h>
#include <chrono>
#pragma comment(lib, "Winhttp.lib")

static bool CrackUrl(const std::wstring& url, URL_COMPONENTS& parts, std::wstring& host, std::wstring& path) {
    ZeroMemory(&parts, sizeof(parts)); parts.dwStructSize = sizeof(parts);
    wchar_t host_buf[512]{}; wchar_t path_buf[2048]{};
    parts.lpszHostName = host_buf; parts.dwHostNameLength = 512;
    parts.lpszUrlPath = path_buf; parts.dwUrlPathLength = 2048;
    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &parts)) return false;
    host.assign(parts.lpszHostName, parts.dwHostNameLength);
    path.assign(parts.lpszUrlPath, parts.dwUrlPathLength);
    if (path.empty()) path = L"/";
    return true;
}

std::vector<HttpCheckResult> HttpChecker::Check(const std::vector<TargetConfig>& targets, int timeout_ms) {
    std::vector<HttpCheckResult> results;
    for (const auto& t : targets) if (t.enabled) results.push_back(CheckOne(t, timeout_ms));
    return results;
}

HttpCheckResult HttpChecker::CheckOne(const TargetConfig& target, int timeout_ms) {
    HttpCheckResult r; r.name = target.name; r.url = target.url;
    URL_COMPONENTS parts{}; std::wstring host, path;
    if (!CrackUrl(target.url, parts, host, path)) { r.error_message = L"Invalid URL"; return r; }
    auto start = std::chrono::steady_clock::now();
    HINTERNET session = WinHttpOpen(L"NetDoctor/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) { r.error_message = L"WinHttpOpen failed"; return r; }
    WinHttpSetTimeouts(session, timeout_ms, timeout_ms, timeout_ms, timeout_ms);
    HINTERNET connect = WinHttpConnect(session, host.c_str(), parts.nPort, 0);
    if (!connect) { r.error_message = L"WinHttpConnect failed"; WinHttpCloseHandle(session); return r; }
    DWORD flags = (parts.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    auto do_request = [&](const wchar_t* method, DWORD& status, DWORD& last_error) -> bool {
        HINTERNET req = WinHttpOpenRequest(connect, method, path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!req) { last_error = GetLastError(); return false; }
        BOOL ok = WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(req, nullptr);
        if (ok) {
            DWORD size = sizeof(status);
            WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &status, &size, nullptr);
        } else {
            last_error = GetLastError();
        }
        WinHttpCloseHandle(req);
        return ok;
    };
    DWORD status = 0, last_error = 0;
    BOOL ok = do_request(L"HEAD", status, last_error);
    if (!ok || status == 405 || status == 403) {
        status = 0; last_error = 0;
        ok = do_request(L"GET", status, last_error);
    }
    auto end = std::chrono::steady_clock::now();
    r.latency_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    if (ok) {
        r.status_code = (int)status;
        r.success = (status >= 200 && status < 400);
        if (!r.success) r.error_message = L"HTTP " + std::to_wstring(status);
    } else {
        r.error_message = L"HTTP request failed: " + std::to_wstring(last_error);
    }
    WinHttpCloseHandle(connect); WinHttpCloseHandle(session);
    return r;
}
