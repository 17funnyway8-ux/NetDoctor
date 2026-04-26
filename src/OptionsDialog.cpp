#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "OptionsDialog.h"
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <string>

#pragma comment(lib, "Comctl32.lib")

namespace OptionsDialog {
namespace {
constexpr int IDC_SAVE = 1001;
constexpr int IDC_CANCEL = 1002;
constexpr int IDC_OPEN_INI = 1003;
constexpr int IDC_RELOAD_HINT = 1004;
constexpr int IDC_BASE = 2000;
constexpr int EDIT_H = 24;
constexpr int PAD = 14;

struct Field {
    const wchar_t* section;
    const wchar_t* key;
    const wchar_t* label;
    const wchar_t* def;
    bool multiline;
    int height;
    int id;
};

Field kFields[] = {
    {L"General", L"CheckIntervalSeconds", L"检测间隔（秒）", L"30", false, EDIT_H, IDC_BASE + 1},
    {L"General", L"TimeoutMilliseconds", L"超时时间（毫秒）", L"3000", false, EDIT_H, IDC_BASE + 2},
    {L"General", L"EnableTooltip", L"启用 Tooltip（0/1）", L"1", false, EDIT_H, IDC_BASE + 3},
    {L"General", L"Language", L"语言", L"zh-CN", false, EDIT_H, IDC_BASE + 4},
    {L"Threshold", L"LatencyGoodMs", L"HTTP 良好阈值 ms", L"100", false, EDIT_H, IDC_BASE + 5},
    {L"Threshold", L"LatencySlowMs", L"HTTP 慢阈值 ms", L"300", false, EDIT_H, IDC_BASE + 6},
    {L"Threshold", L"DnsGoodMs", L"DNS 良好阈值 ms", L"80", false, EDIT_H, IDC_BASE + 7},
    {L"Threshold", L"DnsSlowMs", L"DNS 慢阈值 ms", L"200", false, EDIT_H, IDC_BASE + 8},
    {L"DNS", L"Enabled", L"DNS 启用（0/1）", L"1", false, EDIT_H, IDC_BASE + 9},
    {L"DNS", L"Domains", L"DNS 域名（逗号分隔）", L"www.baidu.com,github.com,cloudflare.com", true, 54, IDC_BASE + 10},
    {L"CN", L"Enabled", L"国内网络启用（0/1）", L"1", false, EDIT_H, IDC_BASE + 11},
    {L"CN", L"Targets", L"国内目标：名称|URL，逗号分隔", L"Baidu|https://www.baidu.com,QQ|https://www.qq.com", true, 54, IDC_BASE + 12},
    {L"International", L"Enabled", L"国际网络启用（0/1）", L"1", false, EDIT_H, IDC_BASE + 13},
    {L"International", L"Targets", L"国际目标：名称|URL，逗号分隔", L"GitHub|https://github.com,Cloudflare|https://www.cloudflare.com", true, 54, IDC_BASE + 14},
    {L"Proxy", L"Enabled", L"代理检测启用（0/1）", L"1", false, EDIT_H, IDC_BASE + 15},
    {L"Proxy", L"DetectSystemProxy", L"检测系统代理（0/1）", L"1", false, EDIT_H, IDC_BASE + 16},
    {L"Proxy", L"Ports", L"本地代理端口：host:port，逗号分隔", L"127.0.0.1:7890,127.0.0.1:7897,127.0.0.1:1080,127.0.0.1:20171", true, 54, IDC_BASE + 17},
    {L"Developer", L"Enabled", L"开发者站点启用（0/1）", L"0", false, EDIT_H, IDC_BASE + 18},
    {L"Developer", L"Targets", L"开发者目标：名称|URL，逗号分隔", L"GitHub|https://github.com,GitHubRaw|https://raw.githubusercontent.com,npm|https://registry.npmjs.org,PyPI|https://pypi.org,Docker|https://hub.docker.com", true, 70, IDC_BASE + 19},
    {L"AI", L"Enabled", L"AI 服务启用（0/1）", L"0", false, EDIT_H, IDC_BASE + 20},
    {L"AI", L"Targets", L"AI 目标：名称|URL，逗号分隔", L"OpenAI|https://api.openai.com,Anthropic|https://api.anthropic.com,Gemini|https://generativelanguage.googleapis.com,HuggingFace|https://huggingface.co,Ollama|http://127.0.0.1:11434,LMStudio|http://127.0.0.1:1234", true, 70, IDC_BASE + 21},
    {L"Home", L"Enabled", L"家庭网络启用（0/1）", L"0", false, EDIT_H, IDC_BASE + 22},
    {L"Home", L"Targets", L"家庭网络目标：名称|URL，逗号分隔", L"Router|http://192.168.1.1,NAS|http://192.168.1.10:5000,HomeAssistant|http://homeassistant.local:8123,Jellyfin|http://127.0.0.1:8096", true, 70, IDC_BASE + 23},
    {L"Ping", L"Enabled", L"Ping 启用（0/1）", L"0", false, EDIT_H, IDC_BASE + 24},
    {L"Ping", L"Hosts", L"Ping 主机（逗号分隔）", L"223.5.5.5,1.1.1.1,github.com", true, 54, IDC_BASE + 25},
    {L"Ping", L"Count", L"Ping 次数（1-10）", L"4", false, EDIT_H, IDC_BASE + 26},
    {L"CustomSites", L"Enabled", L"自定义站点启用（0/1）", L"0", false, EDIT_H, IDC_BASE + 27},
    {L"CustomSites", L"Targets", L"自定义目标：名称|URL，逗号分隔", L"NAS|http://192.168.1.10:5000,Router|http://192.168.1.1,Blog|https://example.com", true, 70, IDC_BASE + 28},
    {L"PublicIP", L"Enabled", L"公网 IP 启用（0/1）", L"0", false, EDIT_H, IDC_BASE + 29},
    {L"PublicIP", L"CheckIntervalSeconds", L"公网 IP 检测间隔（秒）", L"600", false, EDIT_H, IDC_BASE + 30},
    {L"PublicIP", L"Providers", L"公网 IP Provider（逗号分隔）", L"https://api.ipify.org,https://ifconfig.me/ip", true, 54, IDC_BASE + 31},
};

struct DialogState {
    HWND hwnd{};
    HWND scroll{};
    std::wstring path;
    bool saved{false};
    int content_height{0};
    int scroll_pos{0};
    HFONT font{};
};

std::wstring ReadIni(const std::wstring& path, const Field& f) {
    wchar_t buf[8192]{};
    GetPrivateProfileStringW(f.section, f.key, f.def, buf, 8192, path.c_str());
    return buf;
}

std::wstring GetText(HWND h) {
    int len = GetWindowTextLengthW(h);
    std::wstring s(len, L'\0');
    if (len) GetWindowTextW(h, &s[0], len + 1);
    return s;
}

void SetFont(HWND h, HFONT font) { SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE); }

HWND Add(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id, HFONT font, DWORD ex = 0) {
    HWND c = CreateWindowExW(ex, cls, text, style, x, y, w, h, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
    if (font) SetFont(c, font);
    return c;
}

void UpdateScroll(DialogState* st) {
    RECT rc{}; GetClientRect(st->scroll, &rc);
    SCROLLINFO si{ sizeof(si) };
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = st->content_height;
    si.nPage = rc.bottom - rc.top;
    si.nPos = st->scroll_pos;
    SetScrollInfo(st->scroll, SB_VERT, &si, TRUE);
}

void Layout(DialogState* st) {
    RECT rc{}; GetClientRect(st->hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    MoveWindow(st->scroll, PAD, 78, w - PAD * 2, h - 142, TRUE);
    MoveWindow(GetDlgItem(st->hwnd, IDC_OPEN_INI), PAD, h - 50, 120, 32, TRUE);
    MoveWindow(GetDlgItem(st->hwnd, IDC_RELOAD_HINT), PAD + 132, h - 42, w - 430, 24, TRUE);
    MoveWindow(GetDlgItem(st->hwnd, IDC_SAVE), w - 210, h - 50, 90, 32, TRUE);
    MoveWindow(GetDlgItem(st->hwnd, IDC_CANCEL), w - 110, h - 50, 90, 32, TRUE);
    UpdateScroll(st);
}

void CreateFields(DialogState* st) {
    int y = PAD;
    const wchar_t* last = L"";
    int label_w = 230;
    int edit_x = label_w + PAD * 2;
    int edit_w = 620;
    for (auto& f : kFields) {
        if (wcscmp(last, f.section) != 0) {
            Add(st->scroll, L"STATIC", f.section, WS_CHILD | WS_VISIBLE, PAD, y, 820, 22, 0, st->font);
            y += 28;
            last = f.section;
        }
        Add(st->scroll, L"STATIC", f.label, WS_CHILD | WS_VISIBLE, PAD, y + 4, label_w, 22, 0, st->font);
        DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL;
        if (f.multiline) style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL;
        HWND edit = Add(st->scroll, L"EDIT", ReadIni(st->path, f).c_str(), style, edit_x, y, edit_w, f.height, f.id, st->font, WS_EX_CLIENTEDGE);
        SendMessageW(edit, EM_LIMITTEXT, 6000, 0);
        y += f.height + 10;
    }
    st->content_height = y + PAD;
}

bool Save(DialogState* st) {
    for (auto& f : kFields) {
        std::wstring val = GetText(GetDlgItem(st->scroll, f.id));
        if (!WritePrivateProfileStringW(f.section, f.key, val.c_str(), st->path.c_str())) {
            MessageBoxW(st->hwnd, (std::wstring(L"保存失败：") + f.section + L"/" + f.key).c_str(), L"NetDoctor", MB_ICONERROR);
            return false;
        }
    }
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, st->path.c_str());
    st->saved = true;
    MessageBoxW(st->hwnd, L"已保存配置，并将立即重新加载检测。", L"NetDoctor", MB_OK | MB_ICONINFORMATION);
    DestroyWindow(st->hwnd);
    return true;
}

LRESULT CALLBACK ScrollProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR ref) {
    auto* st = reinterpret_cast<DialogState*>(ref);
    switch (msg) {
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wp);
        SendMessageW(hwnd, WM_VSCROLL, delta > 0 ? SB_LINEUP : SB_LINEDOWN, 0);
        return 0;
    }
    case WM_VSCROLL: {
        RECT rc{}; GetClientRect(hwnd, &rc);
        int page = rc.bottom - rc.top;
        int old = st->scroll_pos;
        switch (LOWORD(wp)) {
        case SB_LINEUP: st->scroll_pos -= 32; break;
        case SB_LINEDOWN: st->scroll_pos += 32; break;
        case SB_PAGEUP: st->scroll_pos -= page; break;
        case SB_PAGEDOWN: st->scroll_pos += page; break;
        case SB_THUMBTRACK: case SB_THUMBPOSITION: {
            SCROLLINFO si{ sizeof(si), SIF_TRACKPOS }; GetScrollInfo(hwnd, SB_VERT, &si); st->scroll_pos = si.nTrackPos; break;
        }}
        if (st->scroll_pos < 0) st->scroll_pos = 0;
        int max_pos = st->content_height > page ? st->content_height - page : 0;
        if (st->scroll_pos > max_pos) st->scroll_pos = max_pos;
        if (old != st->scroll_pos) {
            ScrollWindowEx(hwnd, 0, old - st->scroll_pos, nullptr, nullptr, nullptr, nullptr, SW_SCROLLCHILDREN | SW_INVALIDATE);
            UpdateScroll(st);
        }
        return 0;
    }}
    return DefSubclassProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* st = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_CREATE: {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        st = reinterpret_cast<DialogState*>(cs->lpCreateParams);
        st->hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(st));
        NONCLIENTMETRICSW ncm{ sizeof(ncm) }; SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        st->font = CreateFontIndirectW(&ncm.lfMessageFont);
        Add(hwnd, L"STATIC", L"NetDoctor 设置", WS_CHILD | WS_VISIBLE, PAD, 14, 300, 28, 0, st->font);
        Add(hwnd, L"STATIC", L"V1.1 图形化配置；保存后立即 reload，并触发一次后台检测。", WS_CHILD | WS_VISIBLE, PAD, 44, 720, 22, 0, st->font);
        st->scroll = Add(hwnd, L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER, PAD, 78, 880, 500, 0, st->font, WS_EX_CLIENTEDGE);
        SetWindowSubclass(st->scroll, ScrollProc, 1, reinterpret_cast<DWORD_PTR>(st));
        CreateFields(st);
        Add(hwnd, L"BUTTON", L"打开 INI", WS_CHILD | WS_VISIBLE | WS_TABSTOP, PAD, 600, 120, 32, IDC_OPEN_INI, st->font);
        Add(hwnd, L"STATIC", L"提示：0/1 项可直接输入；目标格式为 名称|URL，多项逗号分隔。", WS_CHILD | WS_VISIBLE, PAD + 132, 606, 500, 24, IDC_RELOAD_HINT, st->font);
        Add(hwnd, L"BUTTON", L"保存", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 720, 600, 90, 32, IDC_SAVE, st->font);
        Add(hwnd, L"BUTTON", L"取消", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 820, 600, 90, 32, IDC_CANCEL, st->font);
        Layout(st);
        return 0;
    }
    case WM_SIZE: if (st) Layout(st); return 0;
    case WM_COMMAND:
        if (!st) break;
        switch (LOWORD(wp)) {
        case IDC_SAVE: Save(st); return 0;
        case IDC_CANCEL: DestroyWindow(hwnd); return 0;
        case IDC_OPEN_INI: ShellExecuteW(hwnd, L"open", L"notepad.exe", st->path.c_str(), nullptr, SW_SHOWNORMAL); return 0;
        }
        break;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    case WM_DESTROY:
        if (st && st->font) { DeleteObject(st->font); st->font = nullptr; }
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
}

bool Show(void* hParent, const std::wstring& config_path) {
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES }; InitCommonControlsEx(&icc);
    HINSTANCE inst = GetModuleHandleW(nullptr);
    const wchar_t* cls = L"NetDoctorOptionsWindowV11";
    WNDCLASSEXW wc{ sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = cls;
    RegisterClassExW(&wc);

    DialogState st; st.path = config_path;
    HWND parent = reinterpret_cast<HWND>(hParent);
    HWND hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, cls, L"NetDoctor 设置", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 720, parent, nullptr, inst, &st);
    if (!hwnd) return false;
    if (parent) EnableWindow(parent, FALSE);
    MSG msg{};
    while (IsWindow(hwnd) && GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(hwnd, &msg)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    }
    if (parent) { EnableWindow(parent, TRUE); SetForegroundWindow(parent); }
    return st.saved;
}
}
