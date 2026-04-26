#pragma once
#include <string>

namespace OptionsDialog {
// Native Win32 v1.1 options editor. Returns true when the user saved changes.
bool Show(void* hParent, const std::wstring& config_path);
}
