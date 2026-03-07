#include "framework.h"
#include "Clock.h"
#include "Resource.h"

const wchar_t EXAM_CLASS_NAME[] = L"DesktopExamClockClass";
const wchar_t EXAM_WINDOW_TITLE[] = L"中考倒计时";

const wchar_t CLASS_NAME[] = L"DesktopClockClass";
const wchar_t WINDOW_TITLE[] = L"Desktop Clock";

// x, y, fontSize, Color
ClockConfig g_config = { 0, 2, 36, RGB(255, 255, 255) };

// 主窗口句柄（用于在对话框回调或其他位置触发重绘）
HWND g_hwndMain = NULL;
HWND g_hwndExam = NULL;

// 年级相关，默认为2026年6月15日
int g_examYear = 2026, g_examMonth = 6, g_examDay = 15;
int g_sportExamYear = 2026, g_sportExamMonth = 5, g_sportExamDay = 20;
int g_englishExamYear = 2026, g_englishExamMonth = 5, g_englishExamDay = 25;

// 配置数据结构体（用于存储到 EXE 文件末尾）
struct ConfigData {
    int x;
    int y;
    int fontSize;
    COLORREF textColor;
    int examYear, examMonth, examDay;
    int sportExamYear, sportExamMonth, sportExamDay;
    int englishExamYear, englishExamMonth, englishExamDay;
    int checksum;  // 简单校验和
};

// 计算校验和
int CalcChecksum(const ConfigData* cfg) {
    int sum = 0;
    const char* p = (const char*)cfg;
    for (size_t i = 0; i < sizeof(ConfigData) - sizeof(int); i++) {
        sum += p[i];
    }
    return sum;
}

// 获取配置文件路径
void GetConfigFilePath(wchar_t* path) {
    wchar_t appDataPath[MAX_PATH];

    // 获取用户 AppData\Roaming 目录
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        wcscat_s(appDataPath, L"\\DesktopClock");
        CreateDirectoryW(appDataPath, NULL);
        wcscat_s(appDataPath, L"\\DesktopClockConfig.config");
        wcscpy_s(path, MAX_PATH, appDataPath);
    } else {
        // 如果失败，使用程序所在目录
        GetModuleFileNameW(NULL, path, MAX_PATH);
        wchar_t* p = wcsrchr(path, L'\\');
        if (p) *(p + 1) = L'\0';
        wcscat_s(path, sizeof(path), L"DesktopClockConfig.config");
    }
}

// 从用户目录加载配置
void LoadConfig() {
    OutputDebugStringW(L"[Debug] Entering LoadConfig\n");
    wchar_t configPath[MAX_PATH];
    GetConfigFilePath(configPath);

    FILE* fp;
    if (_wfopen_s(&fp, configPath, L"rb") != 0) {
        OutputDebugStringW(L"[ERR] Failed to open config file\n");
        return;
    }

    ConfigData cfg;
    fread(&cfg, sizeof(ConfigData), 1, fp);
    fclose(fp);

    if (cfg.checksum == CalcChecksum(&cfg)) {
        g_config.x = cfg.x;
        g_config.y = cfg.y;
        g_config.fontSize = cfg.fontSize;
        g_config.textColor = cfg.textColor;
        g_examYear = cfg.examYear;
        g_examMonth = cfg.examMonth;
        g_examDay = cfg.examDay;
        g_sportExamYear = cfg.sportExamYear;
        g_sportExamMonth = cfg.sportExamMonth;
        g_sportExamDay = cfg.sportExamDay;
        g_englishExamYear = cfg.englishExamYear;
        g_englishExamMonth = cfg.englishExamMonth;
        g_englishExamDay = cfg.englishExamDay;
        OutputDebugStringW(L"[Debug] Config loaded successfully\n");
    } else {
        OutputDebugStringW(L"[ERR] Config checksum mismatch\n");
    }
}

// 保存配置到用户目录
void SaveConfig() {
    OutputDebugStringW(L"[Debug] Entering SaveConfig\n");
    wchar_t configPath[MAX_PATH];
    GetConfigFilePath(configPath);

    ConfigData cfg;
    cfg.x = g_config.x;
    cfg.y = g_config.y;
    cfg.fontSize = g_config.fontSize;
    cfg.textColor = g_config.textColor;
    cfg.examYear = g_examYear;
    cfg.examMonth = g_examMonth;
    cfg.examDay = g_examDay;
    cfg.sportExamYear = g_sportExamYear;
    cfg.sportExamMonth = g_sportExamMonth;
    cfg.sportExamDay = g_sportExamDay;
    cfg.englishExamYear = g_englishExamYear;
    cfg.englishExamMonth = g_englishExamMonth;
    cfg.englishExamDay = g_englishExamDay;
    cfg.checksum = CalcChecksum(&cfg);

    FILE* fp;
    if (_wfopen_s(&fp, configPath, L"wb") != 0) {
        OutputDebugStringW(L"[ERR] Failed to open config file for writing\n");
        return;
    }

    fwrite(&cfg, sizeof(ConfigData), 1, fp);
    fclose(fp);
    OutputDebugStringW(L"[Debug] Config saved successfully\n");
}

// 中考倒计时窗口过程
LRESULT CALLBACK ExamWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    wchar_t debugMsg[128];
    swprintf_s(debugMsg, L"[Debug] ExamWndProc: msg=0x%X\n", msg);
    OutputDebugStringW(debugMsg);

    switch (msg) {
    case WM_CREATE:
        OutputDebugStringW(L"[Debug] ExamWndProc: WM_CREATE\n");
        SetTimer(hwnd, 1, 1000, NULL); // 每秒刷新一次
        return 0;
    case WM_TIMER: {
        OutputDebugStringW(L"[Debug] ExamWndProc: WM_TIMER\n");
        // 在 WM_TIMER 中绘制（与主窗口逻辑一致）
        HDC hdcScreen = GetDC(NULL);
        RECT rect;
        GetClientRect(hwnd, &rect);

        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = rect.right;
        bmi.bmiHeader.biHeight = -rect.bottom;  // 负数表示自上而下
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits;
        HBITMAP hBmp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmp);

        // 填充黑色背景
        FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        // 绘制倒计时文本
        SetBkMode(hdcMem, TRANSPARENT);
        SetTextColor(hdcMem, RGB(255, 255, 255));
        HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Arial");
        HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

        wchar_t buffer[64];
        RECT textRect;

        // 1. 体育中考倒计时
        int sportDays = 0;
        try {
            sportDays = GetDaysUntillHighSchoolExam(g_sportExamYear, g_sportExamMonth, g_sportExamDay);
        } catch (...) {
            sportDays = -1;
        }
        textRect = { 0, 0, rect.right, rect.bottom / 3 };
        if (sportDays >= 0) {
            swprintf_s(buffer, L"体育中考: %d 天", sportDays);
        } else {
            swprintf_s(buffer, L"体育中考: 已结束");
        }
        DrawTextW(hdcMem, buffer, -1, &textRect, DT_CENTER | DT_TOP | DT_SINGLELINE);

        // 2. 英语口语中考倒计时
        int englishDays = 0;
        try {
            englishDays = GetDaysUntillHighSchoolExam(g_englishExamYear, g_englishExamMonth, g_englishExamDay);
        } catch (...) {
            englishDays = -1;
        }
        textRect = { 0, rect.bottom / 3, rect.right, rect.bottom * 2 / 3 };
        if (englishDays >= 0) {
            swprintf_s(buffer, L"英语口语: %d 天", englishDays);
        } else {
            swprintf_s(buffer, L"英语口语: 已结束");
        }
        DrawTextW(hdcMem, buffer, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 3. 中考倒计时
        int examDays = 0;
        try {
            examDays = GetDaysUntillHighSchoolExam(g_examYear, g_examMonth, g_examDay);
        } catch (...) {
            examDays = -1;
        }
        textRect = { 0, rect.bottom * 2 / 3, rect.right, rect.bottom };
        if (examDays >= 0) {
            swprintf_s(buffer, L" 中       考: %d 天", examDays);
        } else {
            swprintf_s(buffer, L" 中       考: 已结束");
        }
        DrawTextW(hdcMem, buffer, -1, &textRect, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);

        // 使用 UpdateLayeredWindow 更新窗口
        POINT ptSrc = { 0, 0 };
        SIZE windowSize = { rect.right, rect.bottom };
        BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

        RECT rcWindow;
        GetWindowRect(hwnd, &rcWindow);
        POINT ptDst = { rcWindow.left, rcWindow.top };

        UpdateLayeredWindow(hwnd, hdcScreen, &ptDst, &windowSize,
            hdcMem, &ptSrc, RGB(0, 0, 0), &blend, ULW_COLORKEY);

        // 清理资源
        SelectObject(hdcMem, hOldFont);
        DeleteObject(hFont);
        SelectObject(hdcMem, hOldBmp);
        DeleteObject(hBmp);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        return 0;
    }
    case WM_PAINT: {
        // 分层窗口不需要 BeginPaint/EndPaint，直接返回即
        // 实际绘制由 WM_TIMER 触发 UpdateLayeredWindow 完成
        return 0;
    }
    case WM_DESTROY:
        OutputDebugStringW(L"[Debug] ExamWndProc: WM_DESTROY\n");
        KillTimer(hwnd, 1);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void DrawClockToWindow(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    RECT rect;
    GetClientRect(hwnd, &rect);
    HDC hdcMem = CreateCompatibleDC(hdc);
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = rect.right;
    bmi.bmiHeader.biHeight = -rect.bottom;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    void* pBits;
    HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    SelectObject(hdcMem, hBmp);
    FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    wchar_t timeStr[20];
    swprintf_s(timeStr, L"%02d:%02d:%02d", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    SetTextColor(hdcMem, g_config.textColor);
    SetBkMode(hdcMem, TRANSPARENT);
    HFONT hFont = CreateFont(g_config.fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
    DrawTextW(hdcMem, timeStr, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    POINT ptSrc = { 0, 0 };
    SIZE windowSize = { rect.right, rect.bottom };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, 0 };
    UpdateLayeredWindow(hwnd, hdc, NULL, &windowSize, hdcMem, &ptSrc, RGB(0, 0, 0), &blend, ULW_COLORKEY);
    SelectObject(hdcMem, hOldFont);
    DeleteObject(hFont);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdc);
}

void UpdateExamCountdowns() {
    // 更新中考倒计时窗口
    if (g_hwndExam) {
        InvalidateRect(g_hwndExam, NULL, TRUE);
        SendMessage(g_hwndExam, WM_TIMER, 0, 0);
    }
    OutputDebugStringW(L"[Debug] Function called:UpdateExamCountdowns\n");
}

void HandleTrayMenuCommand(HWND hwnd, UINT cmd) {
    //SetForegroundWindow(hwnd);
    switch (cmd) {
    case IDM_EXIT:
        OutputDebugStringW(L"[Debug] Received IDM_EXIT\n");
        DestroyWindow(hwnd);
        OutputDebugStringW(L"[Debug] Function called:DestroyWindow\n");
        return;
    case IDM_CHANGE_POSITION:
        OutputDebugStringW(L"[Debug] Received IDM_CHANGE_POSITION\n");
        ShowPositionDialog(hwnd);
        return;
    case IDM_CHANGE_FONTSIZE:
        OutputDebugStringW(L"[Debug] Received IDM_CHANGE_FONTSIZE\n");
        ShowFontSizeDialog(hwnd);
        return;
    case IDM_CHANGE_COLOR:
        OutputDebugStringW(L"[Debug] Received IDM_CHANGE_COLOR\n");
        ShowColorDialog(hwnd);
        return;
    case IDM_SET_EXAM_DATE:
        OutputDebugStringW(L"[Debug] Received IDM_SET_EXAM_DATE\n");
        ShowExamDateDialog(hwnd);
        return;
    case IDM_SHOW_BIGSCREEN_T:
        OutputDebugStringW(L"[Debug] Received IDM_SHOW_BIGSCREEN_T\n");
        system("start https://time.is/zh/China");
        return;
    case IDM_ABOUT:
        OutputDebugStringW(L"[Debug] Received IDM_ABOUT\n");
        ShowAboutDialog(hwnd);
        return;
    }
}

// 新增对话框过程：设置倒计时日期
INT_PTR CALLBACK ExamDateDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);

        SYSTEMTIME st = { 0 };

        // 初始化中考日期
        st.wYear = g_examYear;
        st.wMonth = g_examMonth;
        st.wDay = g_examDay;
        DateTime_SetSystemtime(GetDlgItem(hwnd, IDC_EXAM_DATE), GDT_VALID, &st);

        // 初始化体育中考日期
        st.wYear = g_sportExamYear;
        st.wMonth = g_sportExamMonth;
        st.wDay = g_sportExamDay;
        DateTime_SetSystemtime(GetDlgItem(hwnd, IDC_SPORT_DATE), GDT_VALID, &st);

        // 初始化英语口语中考日期
        st.wYear = g_englishExamYear;
        st.wMonth = g_englishExamMonth;
        st.wDay = g_englishExamDay;
        DateTime_SetSystemtime(GetDlgItem(hwnd, IDC_ENGLISH_DATE), GDT_VALID, &st);

        return TRUE;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDOK: {
            SYSTEMTIME st = { 0 };

            // 保存中考日期
            DateTime_GetSystemtime(GetDlgItem(hwnd, IDC_EXAM_DATE), &st);
            g_examYear = st.wYear;
            g_examMonth = st.wMonth;
            g_examDay = st.wDay;

            // 保存体育中考日期
            DateTime_GetSystemtime(GetDlgItem(hwnd, IDC_SPORT_DATE), &st);
            g_sportExamYear = st.wYear;
            g_sportExamMonth = st.wMonth;
            g_sportExamDay = st.wDay;

            // 保存英语口语中考日期
            DateTime_GetSystemtime(GetDlgItem(hwnd, IDC_ENGLISH_DATE), &st);
            g_englishExamYear = st.wYear;
            g_englishExamMonth = st.wMonth;
            g_englishExamDay = st.wDay;

            UpdateExamCountdowns();
            SaveConfig();
            EndDialog(hwnd, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

// 托盘图标相关
NOTIFYICONDATA g_nid = { 0 };

// 添加托盘图标
void AddTrayIcon(HWND hwnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = TRAY_ICON_ID;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = hIcon;
    wcscpy_s(g_nid.szTip, L"Desktop Clock");
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void RemoveTrayIcon(HWND hwnd) {
    g_nid.hWnd = hwnd;
    g_nid.uID = TRAY_ICON_ID;
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
}

void ShowContextMenu(HWND hwnd) {
    HMENU hMenu = CreatePopupMenu();

    AppendMenuW(hMenu, MF_STRING, IDM_CHANGE_POSITION, L"更改位置");
    AppendMenuW(hMenu, MF_STRING, IDM_CHANGE_FONTSIZE, L"更改字体大小");
    AppendMenuW(hMenu, MF_STRING, IDM_CHANGE_COLOR, L"更改颜色");
    AppendMenuW(hMenu, MF_STRING, IDM_SET_EXAM_DATE, L"设置倒计时日期");
    AppendMenuW(hMenu, MF_STRING, IDM_SHOW_BIGSCREEN_T, L"显示大屏时间");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"退出");
    AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"by Chenghao_Kou");

    POINT pt;
    GetCursorPos(&pt);

    // 确保窗口在前台并激活
    SetForegroundWindow(hwnd);

    // 不使用 TPM_RETURNCMD，让菜单命令通过 WM_COMMAND 消息发送到窗口
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

    // 菜单关闭后，确保窗口能正确接收下一个点击事件（解决菜单第二次打不开的问题）
    PostMessageW(hwnd, WM_NULL, 0, 0);

    DestroyMenu(hMenu);
}


INT_PTR CALLBACK PositionDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch (msg) {
    case WM_INITDIALOG: {
        // 强制显示对话框
        ShowWindow(hwnd, SW_SHOW);
        //SetForegroundWindow(hwnd);

        wchar_t buffer[16];
        swprintf_s(buffer, L"%d", g_config.x);
        SetDlgItemTextW(hwnd, IDC_X_EDIT, buffer);
        swprintf_s(buffer, L"%d", g_config.y);
        SetDlgItemTextW(hwnd, IDC_Y_EDIT, buffer);
        return TRUE;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDOK: {
            wchar_t xBuffer[16], yBuffer[16];
            GetDlgItemTextW(hwnd, IDC_X_EDIT, xBuffer, 16);
            GetDlgItemTextW(hwnd, IDC_Y_EDIT, yBuffer, 16);

            g_config.x = _wtoi(xBuffer);
            g_config.y = _wtoi(yBuffer);

            // 使用主窗口句柄更新位置
            UpdateClockPosition(g_hwndMain ? g_hwndMain : GetParent(hwnd), g_config.x, g_config.y);
            SaveConfig();
            EndDialog(hwnd, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK FontSizeDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch (msg) {
    case WM_INITDIALOG: {
        OutputDebugStringW(L"[Debug] FontSizeDialogProc: WM_INITDIALOG\n");
        ShowWindow(hwnd, SW_SHOW);
        //SetForegroundWindow(hwnd);

        wchar_t buffer[16];
        swprintf_s(buffer, L"%d", g_config.fontSize);
        SetDlgItemTextW(hwnd, IDC_FONTSIZE_EDIT, buffer);
        return TRUE;
    }
    case WM_COMMAND: {
        OutputDebugStringW(L"[Debug] FontSizeDialogProc: WM_COMMAND\n");
        switch (LOWORD(wParam)) {
        case IDOK: {
            OutputDebugStringW(L"[Debug] FontSizeDialogProc: IDOK\n");
            wchar_t sizeBuffer[16];
            GetDlgItemTextW(hwnd, IDC_FONTSIZE_EDIT, sizeBuffer, 16);

            int newSize = _wtoi(sizeBuffer);
            if (newSize >= 8 && newSize <= 60) {
                g_config.fontSize = newSize;
                UpdateClockFontSize();
                OutputDebugStringW(L"[Debug] FontSize updated successfully\n");
                SaveConfig();
                EndDialog(hwnd, IDOK);
            } else {
                MessageBoxW(hwnd, L"字体大小必须在8-60之间", L"错误", MB_OK | MB_ICONERROR);
                OutputDebugStringW(L"[Debug] Invalid font size\n");
            }
            return TRUE;
        }
        case IDCANCEL:
            OutputDebugStringW(L"[Debug] FontSizeDialogProc: IDCANCEL\n");
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE:
        OutputDebugStringW(L"[Debug] FontSizeDialogProc: WM_CLOSE\n");
        EndDialog(hwnd, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

INT_PTR CALLBACK AboutDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch (msg) {
    case WM_INITDIALOG:
        ShowWindow(hwnd, SW_SHOW);
        //SetForegroundWindow(hwnd);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}



// 显示位置设置对话框
BOOL ShowPositionDialog(HWND hwnd) {
    return DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_CHANGE_POSITION),
        hwnd,
        PositionDialogProc) == IDOK;
}

// 显示字体大小设置对话框
BOOL ShowFontSizeDialog(HWND hwnd) {
    OutputDebugStringW(L"[Debug] Entering ShowFontSizeDialog\n");
    BOOL result = DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_CHANGE_SIZE),
        hwnd,
        FontSizeDialogProc) == IDOK;
    if (result) {
        OutputDebugStringW(L"[Debug] ShowFontSizeDialog returned IDOK\n");
    } else {
        OutputDebugStringW(L"[Debug] ShowFontSizeDialog returned IDCANCEL or failed\n");
    }
    return result;
}

// 显示颜色选择对话框
BOOL ShowColorDialog(HWND hwnd) {
    OutputDebugStringW(L"[Debug] ShowColorDialog\n");

    CHOOSECOLOR cc = { 0 };
    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.hwndOwner = hwnd;
    cc.rgbResult = g_config.textColor;
    cc.lpCustColors = (LPDWORD)malloc(16 * sizeof(DWORD));
    cc.Flags = CC_RGBINIT | CC_FULLOPEN;

    // 激活窗口
    ShowWindow(hwnd, SW_SHOW);
    //SetForegroundWindow(hwnd);

    if (ChooseColor(&cc)) {
        if (cc.rgbResult == RGB(0, 0, 0)) {
            cc.rgbResult = RGB(0, 0, 1);
        }
        g_config.textColor = cc.rgbResult;
        UpdateClockColor();
        SaveConfig();
        free(cc.lpCustColors);
        return TRUE;
    }

    free(cc.lpCustColors);
    return FALSE;
}

// 显示关于对话框
BOOL ShowAboutDialog(HWND hwnd) {
    OutputDebugStringW(L"[Debug] ShowAboutDialog\n");
    return DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_ABOUTBOX),
        hwnd,
        AboutDialogProc) == IDOK;
}

// 显示设置倒计时日期对话框
BOOL ShowExamDateDialog(HWND hwnd) {
    OutputDebugStringW(L"[Debug] ShowExamDateDialog\n");
    return DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_SET_EXAM_DATE),
        hwnd,
        ExamDateDialogProc) == IDOK;
}

// 更新时钟位置
void UpdateClockPosition(HWND hwnd, int x, int y) {
    SetWindowPos(hwnd, HWND_TOPMOST, x, y, 200, 50, SWP_NOACTIVATE | SWP_NOZORDER);
    InvalidateRect(hwnd, NULL, TRUE);
}

// 更新字体大小
void UpdateClockFontSize() {
    // 使用主窗口句柄触发重绘
    HWND hwnd = g_hwndMain ? g_hwndMain : FindWindowW(CLASS_NAME, WINDOW_TITLE);
    if (hwnd) {
        InvalidateRect(hwnd, NULL, TRUE);
        SendMessage(hwnd, WM_TIMER, 0, 0);
    }
}

// 更新颜色
void UpdateClockColor() {
    HWND hwnd = g_hwndMain ? g_hwndMain : FindWindowW(CLASS_NAME, WINDOW_TITLE);
    if (hwnd) {
        InvalidateRect(hwnd, NULL, TRUE);
        SendMessage(hwnd, WM_TIMER, 0, 0);
    }
}

// 主窗口过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 1000, NULL);
        return 0;
    case WM_TIMER:
        DrawClockToWindow(hwnd);
        return 0;
    case WM_PAINT:
        DrawClockToWindow(hwnd);
        return 0;
    case WM_RBUTTONUP:
        ShowContextMenu(hwnd);
        return 0;
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP) {
            // 使用 PostMessage 延迟显示菜单，确保托盘图标的点击消息处理完毕
            PostMessage(hwnd, WM_USER + 100, 0, 0);
        }
        return 0;
    case WM_USER + 100:  // 延迟处理托盘菜单
        ShowContextMenu(hwnd);
        return 0;
    case WM_COMMAND:
        OutputDebugStringW(L"[Debug] WndProc:WM_COMMAND\n");
        HandleTrayMenuCommand(hwnd, LOWORD(wParam));
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        RemoveTrayIcon(hwnd);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 加载配置（从 EXE 文件末尾读取）
    LoadConfig();

    // 在WinMain开始处加载图标
    hIcon = (HICON)LoadImage(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDI_CLOCK),
        IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);

    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    // 注册中考倒计时窗口类
    WNDCLASSW examWc = { 0 };
    examWc.lpfnWndProc = ExamWndProc;
    examWc.hInstance = hInstance;
    examWc.lpszClassName = EXAM_CLASS_NAME;
    examWc.hIcon = hIcon;
    examWc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&examWc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    g_config.x = screenWidth / 2 - 100;

    HWND hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        CLASS_NAME,
        WINDOW_TITLE,
        WS_POPUP,
        g_config.x, g_config.y,
        200, 50,
        NULL,
        NULL,
        hInstance,
        NULL);


    // 创建中考倒计时窗口，位置在右上角
    int examWidth = 280, examHeight = 72;
    HWND ExamHwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        EXAM_CLASS_NAME,
        EXAM_WINDOW_TITLE,
        WS_POPUP,
        screenWidth - examWidth, 6,
        examWidth, examHeight,
        NULL,
        NULL,
        hInstance,
        NULL);
    SetWindowPos(ExamHwnd, HWND_DESKTOP, screenWidth - examWidth, 0, examWidth, examHeight, SWP_SHOWWINDOW);

    // 保存主窗口句柄
    g_hwndMain = hwnd;
    g_hwndExam = ExamHwnd;

    // 显示主窗口
    ShowWindow(hwnd, SW_SHOW);
    SetWindowPos(hwnd, HWND_TOPMOST, g_config.x, g_config.y, 200, 50, SWP_NOMOVE | SWP_NOSIZE);
    SetTimer(hwnd, 1, 1000, NULL);
    AddTrayIcon(hwnd);

    // 显示倒计时窗口
    ShowWindow(ExamHwnd, SW_SHOW);
    SetWindowPos(ExamHwnd, HWND_DESKTOP, screenWidth - examWidth, 0, examWidth, examHeight, SWP_SHOWWINDOW);

    // 立即触发第一次绘制
    SendMessage(hwnd, WM_TIMER, 0, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}