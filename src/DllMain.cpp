#include "NetDoctorPlugin.h"
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID) { return TRUE; }

extern "C" __declspec(dllexport) ITMPlugin* TMPluginGetInstance()
{
    return &CNetDoctorPlugin::Instance();
}
