#pragma once

#include "resource.h"

HICON hIcon;

// 配置结构体
struct ClockConfig {
	int x;
	int y;
	int fontSize;
	COLORREF textColor;
};

// 菜单命令ID
#define IDM_EXIT 10001
#define IDM_CHANGE_POSITION 10002
#define IDM_CHANGE_FONTSIZE 10003
#define IDM_CHANGE_COLOR 10004
#define IDI_APP_ICON 100

// 托盘图标ID和消息
#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ComCtl32.lib")

// 函数声明
void AddTrayIcon(HWND hwnd);
void RemoveTrayIcon(HWND hwnd);
void ShowContextMenu(HWND hwnd);
BOOL ShowPositionDialog(HWND hwnd);
BOOL ShowFontSizeDialog(HWND hwnd);
BOOL ShowColorDialog(HWND hwnd);
BOOL ShowAboutDialog(HWND hwnd);
void UpdateClockColor();
void UpdateClockPosition(HWND hwnd, int x, int y);
void UpdateClockFontSize();

// 对话框过程声明
INT_PTR CALLBACK PositionDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FontSizeDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
