#pragma once
#include <string>

namespace OptionsDialog {
// Minimal v1.0 options entry: open the INI in Notepad for editing.
// A full graphical editor can replace this without touching the plugin API surface.
bool OpenConfigFile(void* hParent, const std::wstring& config_path);
}
