#include "Utils.h"
#include <windows.h>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Utils {
std::wstring Trim(const std::wstring& s) {
    const wchar_t* ws = L" \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::wstring::npos) return L"";
    auto e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}
std::vector<std::wstring> Split(const std::wstring& s, wchar_t delimiter) {
    std::vector<std::wstring> out; std::wstringstream ss(s); std::wstring item;
    while (std::getline(ss, item, delimiter)) { item = Trim(item); if (!item.empty()) out.push_back(item); }
    return out;
}
std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring r(n, 0); MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), r.data(), n); return r;
}
std::string WideToUtf8(const std::wstring& s) {
    if (s.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string r(n, 0); WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), r.data(), n, nullptr, nullptr); return r;
}
std::wstring FormatTimeNow() {
    auto now = std::chrono::system_clock::now(); auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{}; localtime_s(&tm, &t); std::wstringstream ss; ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S"); return ss.str();
}
std::wstring StatusToText(int ms, bool ok, int good_ms, int slow_ms, const wchar_t* prefix) {
    if (!ok) return std::wstring(prefix) + L" BAD";
    if (ms < 0) return std::wstring(prefix) + L" OK";
    if (ms >= slow_ms) return std::wstring(prefix) + L" SLOW";
    std::wstringstream ss; ss << prefix << L" " << ms << L"ms"; return ss.str();
}
std::wstring GetModuleDirectory() {
    wchar_t path[MAX_PATH]{}; HMODULE hm = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&GetModuleDirectory), &hm);
    GetModuleFileNameW(hm, path, MAX_PATH);
    std::wstring p(path); auto pos = p.find_last_of(L"\\/"); return pos == std::wstring::npos ? L"." : p.substr(0, pos);
}
std::wstring JoinPath(const std::wstring& a, const std::wstring& b) {
    if (a.empty()) return b; wchar_t last = a.back(); if (last == L'\\' || last == L'/') return a + b; return a + L"\\" + b;
}
bool EnsureDirectory(const std::wstring& path) {
    if (CreateDirectoryW(path.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS) return true;
    return false;
}
}
