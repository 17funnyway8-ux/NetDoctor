#include "OptionsDialog.h"
#include <windows.h>
#include <shellapi.h>

namespace OptionsDialog {
bool OpenConfigFile(void* hParent, const std::wstring& config_path) {
    HINSTANCE ret = ShellExecuteW(reinterpret_cast<HWND>(hParent), L"open", L"notepad.exe", config_path.c_str(), nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<intptr_t>(ret) > 32;
}
}
