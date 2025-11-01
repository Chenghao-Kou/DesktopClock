#include "framework.h"
#include "Clock.h"

const wchar_t CLASS_NAME[] = L"DesktopClockClass";
const wchar_t WINDOW_TITLE[] = L"Desktop Clock";

// x, y, fontSize, Color
ClockConfig g_config = { 0, 2, 36, RGB(255, 255, 255) };

// 对话框控件ID
#define IDC_X_EDIT 1001
#define IDC_Y_EDIT 1002
#define IDC_FONTSIZE_EDIT 1003
#define IDC_COLOR_BUTTON 1004
#define IDOK_BUTTON 1005
#define IDCANCEL_BUTTON 1006

// 托盘图标ID
#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)

// 函数声明
void AddTrayIcon(HWND hwnd);
void RemoveTrayIcon(HWND hwnd);
void ShowContextMenu(HWND hwnd);
BOOL ShowPositionDialog(HWND hwnd);
BOOL ShowFontSizeDialog(HWND hwnd);
BOOL ShowColorDialog(HWND hwnd);
BOOL ShowAboutDialog(HWND hwnd);

// 关于对话框过程声明
INT_PTR CALLBACK AboutDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 关于对话框实现（使用资源中的对话框IDD_ABOUTBOX）
BOOL ShowAboutDialog(HWND hwnd) {
    return DialogBox(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_ABOUTBOX),
        hwnd,
        AboutDialogProc) == IDOK;
}

// 关于对话框过程
INT_PTR CALLBACK AboutDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
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

void UpdateClockPosition(HWND hwnd, int x, int y);
void UpdateClockFontSize();
void UpdateClockColor();

// 位置设置对话框过程
INT_PTR CALLBACK PositionDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG: {
		wchar_t buffer[16];
		swprintf_s(buffer, L"%d", g_config.x);
		SetDlgItemTextW(hwnd, IDC_X_EDIT, buffer);
		swprintf_s(buffer, L"%d", g_config.y);
		SetDlgItemTextW(hwnd, IDC_Y_EDIT, buffer);
		return TRUE;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDOK_BUTTON: {
			wchar_t xBuffer[16], yBuffer[16];
			GetDlgItemTextW(hwnd, IDC_X_EDIT, xBuffer, 16);
			GetDlgItemTextW(hwnd, IDC_Y_EDIT, yBuffer, 16);

			g_config.x = _wtoi(xBuffer);
			g_config.y = _wtoi(yBuffer);

			UpdateClockPosition(GetParent(hwnd), g_config.x, g_config.y);
			EndDialog(hwnd, IDOK);
			return TRUE;
		}
		case IDCANCEL_BUTTON: {
			EndDialog(hwnd, IDCANCEL);
			return TRUE;
		}
		}
		break;
	}
	case WM_CLOSE: {
		EndDialog(hwnd, IDCANCEL);
		break;
	}
	}
	return FALSE;
}

// 字体大小设置对话框过程
INT_PTR CALLBACK FontSizeDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG: {
		wchar_t buffer[16];
		swprintf_s(buffer, L"%d", g_config.fontSize);
		SetDlgItemTextW(hwnd, IDC_FONTSIZE_EDIT, buffer);
		return TRUE;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDOK_BUTTON: {
			wchar_t sizeBuffer[16];
			GetDlgItemTextW(hwnd, IDC_FONTSIZE_EDIT, sizeBuffer, 16);

			int newSize = _wtoi(sizeBuffer);
			if (newSize >= 8 && newSize <= 72) {
				g_config.fontSize = newSize;
				UpdateClockFontSize();
				EndDialog(hwnd, IDOK);
			}
			else {
				MessageBoxW(hwnd, L"字体大小必须在8-72之间", L"错误", MB_OK | MB_ICONERROR);
			}
			return TRUE;
		}
		case IDCANCEL_BUTTON: {
			EndDialog(hwnd, IDCANCEL);
			return TRUE;
		}
		}
		break;
	}
	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		break;
	}
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		AddTrayIcon(hwnd);
		return 0;
	}
	case WM_TIMER: {
		HDC hdc = GetDC(hwnd);
		RECT rect;
		GetClientRect(hwnd, &rect);

		// 创建内存DC和32位位图
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

		// 使用黑色填充背景
		FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

		// 获取当前时间
		time_t now = time(0);
		tm ltm;
		localtime_s(&ltm, &now);
		wchar_t timeStr[20];
		swprintf_s(timeStr, L"%02d:%02d:%02d", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);

		// 设置字体和颜色
		SetTextColor(hdcMem, g_config.textColor);
		SetBkMode(hdcMem, TRANSPARENT);
		HFONT hFont = CreateFont(g_config.fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
			DEFAULT_PITCH, L"Arial");
		HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

		// 绘制文本
		DrawTextW(hdcMem, timeStr, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		// 更新分层窗口
		POINT ptSrc = { 0, 0 };
		SIZE windowSize = { rect.right, rect.bottom };
		BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, 0 };
		UpdateLayeredWindow(hwnd, hdc, NULL, &windowSize, hdcMem, &ptSrc, RGB(0, 0, 0), &blend, ULW_COLORKEY);

		// 清理资源
		SelectObject(hdcMem, hOldFont);
		DeleteObject(hFont);
		DeleteObject(hBmp);
		DeleteDC(hdcMem);
		ReleaseDC(hwnd, hdc);
		return 0;
	}
	case WM_TRAYICON: {
		switch (lParam) {
		case WM_RBUTTONUP:
			ShowContextMenu(hwnd);
			break;
		case WM_CONTEXTMENU:
			ShowContextMenu(hwnd);
			break;
		case WM_LBUTTONDBLCLK:
			ShowContextMenu(hwnd);
			break;
		case WM_LBUTTONDOWN:
			ShowContextMenu(hwnd);
			break;
		}
		return 0;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDM_EXIT:
			DestroyWindow(hwnd);
			break;
		case IDM_CHANGE_POSITION:
			ShowPositionDialog(hwnd);
			break;
		case IDM_CHANGE_FONTSIZE:
			ShowFontSizeDialog(hwnd);
			break;
		case IDM_CHANGE_COLOR:
			ShowColorDialog(hwnd);
			break;
		case IDM_ABOUT:
			ShowAboutDialog(hwnd);
			break;
		}
		return 0;
	}
	case WM_DESTROY:
		RemoveTrayIcon(hwnd);
		KillTimer(hwnd, 1);
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

// 添加托盘图标
void AddTrayIcon(HWND hwnd) {
	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = TRAY_ICON_ID;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON;
	nid.hIcon = hIcon;
	wcscpy_s(nid.szTip, L"桌面时钟");

	Shell_NotifyIcon(NIM_ADD, &nid);
}

// 移除托盘图标
void RemoveTrayIcon(HWND hwnd) {
	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = TRAY_ICON_ID;

	Shell_NotifyIcon(NIM_DELETE, &nid);
}

// 显示上下文菜单
void ShowContextMenu(HWND hwnd) {
	HMENU hMenu = CreatePopupMenu();

	AppendMenuW(hMenu, MF_STRING, IDM_CHANGE_POSITION, L"更改位置");
	AppendMenuW(hMenu, MF_STRING, IDM_CHANGE_FONTSIZE, L"更改字体大小");
	AppendMenuW(hMenu, MF_STRING, IDM_CHANGE_COLOR, L"更改颜色");
	AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"退出");
	AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"by寇埕豪");

	POINT pt;
	GetCursorPos(&pt);

	SetForegroundWindow(hwnd);
	TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
	PostMessageW(hwnd, WM_NULL, 0, 0);

	DestroyMenu(hMenu);
}

// 显示位置设置对话框
BOOL ShowPositionDialog(HWND hwnd) {
	return DialogBox(GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_CHANGE_COLOR),
		hwnd,
		PositionDialogProc) == IDOK;
}

// 显示字体大小设置对话框
BOOL ShowFontSizeDialog(HWND hwnd) {
	// 创建简单的输入对话框
	wchar_t buffer[16];
	swprintf_s(buffer, L"%d", g_config.fontSize);

	wchar_t newSize[16] = { 0 };

	if (DialogBoxParam(GetModuleHandle(NULL),
		MAKEINTRESOURCE(2), // 需要创建对话框资源
		hwnd,
		FontSizeDialogProc,
		(LPARAM)newSize) == IDOK) {
		int size = _wtoi(newSize);
		if (size >= 8 && size <= 72) {
			g_config.fontSize = size;
			UpdateClockFontSize();
			return TRUE;
		}
	}
	return FALSE;
}

// 显示颜色选择对话框
BOOL ShowColorDialog(HWND hwnd) {
	CHOOSECOLOR cc = { 0 };
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hwnd;
	cc.rgbResult = g_config.textColor;
	cc.lpCustColors = (LPDWORD)malloc(16 * sizeof(DWORD));
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;

	if (ChooseColor(&cc)) {
		g_config.textColor = cc.rgbResult;
		UpdateClockColor();
		free(cc.lpCustColors);
		return TRUE;
	}

	free(cc.lpCustColors);
	return FALSE;
}

// 更新时钟位置
void UpdateClockPosition(HWND hwnd, int x, int y) {
	SetWindowPos(hwnd, HWND_TOPMOST, x, y, 200, 50, SWP_NOACTIVATE | SWP_NOZORDER);
	InvalidateRect(hwnd, NULL, TRUE);
}

// 更新字体大小
void UpdateClockFontSize() {
	// 字体大小会在下一次WM_TIMER消息时自动应用
}

// 更新颜色
void UpdateClockColor() {
	// 颜色会在下一次WM_TIMER消息时自动应用
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
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

	// 显示窗口
	ShowWindow(hwnd, SW_SHOW);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetTimer(hwnd, 1, 1000, NULL);

	// 立即触发第一次绘制
	SendMessage(hwnd, WM_TIMER, 0, 0);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}