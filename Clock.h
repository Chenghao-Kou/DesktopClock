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
#define IDM_SET_EXAM_DATE 10005
#define IDM_SHOW_BIGSCREEN_T 10006
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
void HandleTrayMenuCommand(HWND hwnd, UINT cmd);
BOOL ShowPositionDialog(HWND hwnd);
BOOL ShowFontSizeDialog(HWND hwnd);
BOOL ShowColorDialog(HWND hwnd);
BOOL ShowAboutDialog(HWND hwnd);
BOOL ShowExamDateDialog(HWND hwnd);
void UpdateClockColor();
void UpdateClockPosition(HWND hwnd, int x, int y);
void UpdateClockFontSize();
void UpdateExamCountdowns();
void DrawClockToWindow(HWND hwnd);

// 对话框过程声明
INT_PTR CALLBACK PositionDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FontSizeDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ExamDateDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 获取距离中考一共还剩几天？  参数：哪一年中考 中考的月份、日期
int GetDaysUntillHighSchoolExam(int Year, int Month, int Day){
    using namespace std::chrono;

    // 获取当前系统时间点
    auto now = system_clock::now();
    // 转换为本地时区的时间点
    auto local_now = current_zone()->to_local(now);
    // 向下取整到「天」，得到今天的本地日期
    local_days today = floor<days>(local_now);

    // 构造中考日期并校验合法性
    year_month_day exam_date = year(Year) / month(Month) / day(Day);
    if (!exam_date.ok()) {
        throw std::invalid_argument(
            std::format("非法的中考日期：{}-{}-{}", Year, Month, Day)
        );
    }

    // 转换为「本地天数」类型计算
    local_days exam_days = local_days(exam_date);

    // 计算天数差并返回整数
    return (exam_days - today).count();
}