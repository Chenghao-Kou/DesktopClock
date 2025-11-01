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

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ComCtl32.lib")