#pragma once
#include <string>
#include <vector>

namespace Utils {
std::wstring Trim(const std::wstring& s);
std::vector<std::wstring> Split(const std::wstring& s, wchar_t delimiter);
std::wstring Utf8ToWide(const std::string& s);
std::string WideToUtf8(const std::wstring& s);
std::wstring FormatTimeNow();
std::wstring StatusToText(int ms, bool ok, int good_ms, int slow_ms, const wchar_t* prefix);
std::wstring GetModuleDirectory();
std::wstring JoinPath(const std::wstring& a, const std::wstring& b);
bool EnsureDirectory(const std::wstring& path);
}
