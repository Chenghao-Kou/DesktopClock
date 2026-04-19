#pragma once

#define VER "3.0.0"

#include "resource.h"
#include <iostream>

HICON hIcon;

// 配置数据结构体
struct ConfigData {
    int x;
    int y;
    int fontSize;
    COLORREF textColor;
    int examYear, examMonth, examDay;
    int sportExamMonth, sportExamDay; // 不需要年份（同一年）
    int englishExamMonth, englishExamDay;
    bool ShowExamDate;
    int checksum;  // 校验和
};

// 菜单命令ID
#define IDM_EXIT 10001
#define IDM_CHANGE_POSITION 10002
#define IDM_CHANGE_FONTSIZE 10003
#define IDM_CHANGE_COLOR 10004
#define IDM_SET_EXAM_DATE 10005
#define IDM_SHOW_BIGSCREEN_T 10006
#define IDM_SHOW_EXAM_DATE 10007
#define IDI_APP_ICON 100


// 托盘图标ID和消息
#define TRAY_ICON_ID 1
#define WM_TRAYICON (WM_USER + 1)

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "wininet.lib")

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


// 从字符串中提取数字版本号（例如 "2.2.3_Stable" -> "2.2.3"）
std::string ExtractNumberVersion(const std::string& tag) {
    std::string result;
    for (char c : tag) {
        if (isdigit(c) || c == '.') {
            result += c;
        } else {
            break;  // 遇到非数字或 '.' 就停止
        }
    }
    return result;
}

// 比较两个版本号字符串（格式 "x.y.z"），如果 v1 < v2 返回 true
bool IsVersionLess(const std::string& v1, const std::string& v2) {
    auto split = [] (const std::string& v) -> std::vector<int> {
        std::vector<int> parts;
        size_t start = 0, end = 0;
        while ((end = v.find('.', start)) != std::string::npos) {
            parts.push_back(std::stoi(v.substr(start, end - start)));
            start = end + 1;
        }
        parts.push_back(std::stoi(v.substr(start)));
        return parts;
        };
    auto p1 = split(v1);
    auto p2 = split(v2);
    size_t maxLen = std::max(p1.size(), p2.size()); //#define NOMINMAX
    for (size_t i = 0; i < maxLen; ++i) {
        int a = i < p1.size() ? p1[i] : 0;
        int b = i < p2.size() ? p2[i] : 0;
        if (a != b) return a < b;
    }
    return false; // 相等
}

// 获取 GitHub 仓库最新 Release 的 JSON 字符串
// owner: 仓库拥有者
// repo:  仓库名
// 返回值：成功返回 API 响应内容（JSON），失败返回空字符串
std::string GetGitHubLatestRelease(const std::wstring& owner, const std::wstring& repo) {
    HINTERNET hInternet = nullptr;
    HINTERNET hConnect = nullptr;
    HINTERNET hRequest = nullptr;
    std::string response;

    // 1. 初始化 WinINet
    hInternet = InternetOpenW(L"MyApp/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        OutputDebugStringW(L"WinINet InternetOpenW failed\n");
        return "";
    }

    // 2. 连接到 GitHub API 服务器 (HTTPS 端口 443)
    hConnect = InternetConnectW(hInternet, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT,
        NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        OutputDebugStringW(L"WinINet InternetConnectW failed (step2)\n");
        InternetCloseHandle(hInternet);
        return "";
    }

    // 3. 构造请求路径
    std::wstring path = L"/repos/" + owner + L"/" + repo + L"/releases/latest";

    // 4. 打开 HTTP 请求（使用 INTERNET_FLAG_SECURE 启用 TLS）
    hRequest = HttpOpenRequestW(hConnect, L"GET", path.c_str(), NULL, NULL, NULL,
        INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        OutputDebugStringW(L"HttpOpenRequestW failed\n");
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    // 5. 发送请求
    if (!HttpSendRequestW(hRequest, NULL, 0, NULL, 0)) {
        OutputDebugStringW(L"HttpSendRequestW failed\n");
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    // 6. 读取响应数据
    char buffer[4096];
    DWORD bytesRead = 0;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead != 0) {
        buffer[bytesRead] = '\0';
        response.append(buffer, bytesRead);
    }

    // 7. 清理资源
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
}

// 将 string 转换为 wstring（UTF-8 安全，用于调试输出）
std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    std::wstring result(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &result[0], len);
    return result;
}

// 下载文件（支持 HTTPS 和重定向）
bool DownloadFile(const std::wstring& url, const std::wstring& destPath) {
    OutputDebugStringW((L"[Download] Start: " + url + L"\n").c_str());

    // 1. 解析 URL 得到主机名和路径
    URL_COMPONENTSW urlComp = { 0 };
    urlComp.dwStructSize = sizeof(urlComp);
    wchar_t hostName[256] = { 0 };
    wchar_t urlPath[1024] = { 0 };
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = ARRAYSIZE(hostName);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = ARRAYSIZE(urlPath);
    urlComp.dwSchemeLength = 0;
    if (!InternetCrackUrlW(url.c_str(), 0, 0, &urlComp)) {
        OutputDebugStringW(L"[Download] Failed to crack URL\n");
        return false;
    }

    BOOL isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);
    DWORD port = isHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

    // 2. 初始化 WinINet
    HINTERNET hInternet = InternetOpenW(L"MyApp/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        OutputDebugStringW(L"[Download] InternetOpenW failed\n");
        return false;
    }

    // 3. 连接服务器
    HINTERNET hConnect = InternetConnectW(hInternet, hostName, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        OutputDebugStringW(L"[Download] InternetConnectW failed\n");
        InternetCloseHandle(hInternet);
        return false;
    }

    // 4. 打开请求（必须加上 INTERNET_FLAG_SECURE 支持 HTTPS）
    DWORD flags = INTERNET_FLAG_RELOAD | (isHttps ? INTERNET_FLAG_SECURE : 0);
    HINTERNET hRequest = HttpOpenRequestW(hConnect, L"GET", urlPath, NULL, NULL, NULL, flags, 0);
    if (!hRequest) {
        OutputDebugStringW(L"[Download] HttpOpenRequestW failed\n");
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    // 5. 设置 User-Agent（避免被某些 CDN 拒绝）
    LPCWSTR userAgent = L"MyApp/1.0";
    HttpAddRequestHeadersW(hRequest, L"User-Agent: MyApp/1.0\r\n", -1L, HTTP_ADDREQ_FLAG_ADD);

    // 6. 发送请求
    if (!HttpSendRequestW(hRequest, NULL, 0, NULL, 0)) {
        OutputDebugStringW(L"[Download] HttpSendRequestW failed\n");
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    // 7. 获取状态码
    DWORD statusCode = 0;
    DWORD size = sizeof(statusCode);
    HttpQueryInfoW(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &size, NULL);
    OutputDebugStringW((L"[Download] HTTP Status: " + std::to_wstring(statusCode) + L"\n").c_str());

    // 8. 处理重定向（301/302/307/308）
    if (statusCode >= 300 && statusCode < 400) {
        wchar_t location[1024] = { 0 };
        DWORD locSize = sizeof(location);
        if (HttpQueryInfoW(hRequest, HTTP_QUERY_LOCATION, location, &locSize, NULL)) {
            OutputDebugStringW((L"[Download] Redirect to: " + std::wstring(location) + L"\n").c_str());
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            // 递归下载重定向后的 URL
            return DownloadFile(location, destPath);
        } else {
            OutputDebugStringW(L"[Download] Failed to get redirect location\n");
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
    }

    if (statusCode != 200) {
        OutputDebugStringW((L"[Download] Unexpected status code: " + std::to_wstring(statusCode) + L"\n").c_str());
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    // 9. 读取数据并写入文件
    HANDLE hFile = CreateFileW(destPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        OutputDebugStringW(L"[Download] Failed to create local file\n");
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead = 0;
    DWORD totalWritten = 0;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
        DWORD bytesWritten = 0;
        WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL);
        totalWritten += bytesWritten;
    }

    CloseHandle(hFile);
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    OutputDebugStringW((L"[Download] Success, total bytes: " + std::to_wstring(totalWritten) + L"\n").c_str());
    return totalWritten > 0;
}

// 替换当前程序并重启
// newExePath: 下载好的新版本临时文件路径（例如 L"C:\\Users\\xxx\\AppData\\Local\\Temp\\app_new.exe"）
// 注意：调用此函数后，当前进程会立即退出，不会再执行后续代码
void ReplaceAndRestart(const std::wstring& newExePath) {
    // 1. 获取当前程序的完整路径
    wchar_t currentExe[MAX_PATH];
    GetModuleFileNameW(NULL, currentExe, MAX_PATH);

    // 2. 获取临时目录路径，用于存放批处理脚本
    wchar_t tempDir[MAX_PATH];
    GetTempPathW(MAX_PATH, tempDir);
    std::wstring scriptPath = std::wstring(tempDir) + L"update.bat";

    // 3. 生成批处理脚本内容
    std::wofstream script(scriptPath);
    if (!script.is_open()) return;

    script << L"@echo off\n";
    script << L"title Updating...\n";
    script << L"echo Waiting for program to exit...\n";
    // 等待2秒，确保主程序完全退出
    script << L"timeout /t 2 /nobreak >nul\n";
    // 备份旧文件（可选，后缀.old）
    //script << L"if exist \"" << currentExe << L".old\" del \"" << currentExe << L".old\"\n";
    //script << L"move /Y \"" << currentExe << L"\" \"" << currentExe << L".old\"\n";
    // 复制新文件到原位置
    script << L"copy /Y \"" << newExePath << L"\" \"" << currentExe << L"\"\n";
    // 删除临时文件
    script << L"del \"" << newExePath << L"\"\n";
    // 重新启动主程序
    script << L"start \"\" \"" << currentExe << L"\"\n";
    // 删除脚本自身
    script << L"del \"%~f0\"\n";
    script.close();

    // 4. 以隐藏窗口方式启动批处理脚本
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi;
    std::wstring cmdLine = L"cmd.exe /c \"" + scriptPath + L"\"";

    CreateProcessW(NULL, (LPWSTR)cmdLine.c_str(), NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    // 关闭句柄（不需要等待）
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // 5. 立即退出当前进程
    ExitProcess(0);
}

void CheckForUpdate() {
    try {
        std::string data = GetGitHubLatestRelease(L"Chenghao-Kou", L"DesktopClock");
        if (data.empty()) {
            MessageBoxW(nullptr, L"无法获取更新信息！请检查网络。", L"错误", MB_ICONERROR);
            return;
        }

        using json = nlohmann::json;
        auto info = json::parse(data);

        // 获取远端版本号并提取数字部分
        std::string remote_tag = info["tag_name"].get<std::string>();
        std::string remote_version = ExtractNumberVersion(remote_tag);
        bool is_prerelease = info["prerelease"].get<bool>();

        // 当前版本（宏定义）
        std::string current_version = VER;

        // 版本比较：只更新正式版且远端版本大于当前版本
        if (is_prerelease) {
            // 如果是预发布版，不更新（可根据需要决定是否允许）
            OutputDebugStringW(L"[Update] Skipping prerelease version\n");
            return;
        }

        if (!IsVersionLess(current_version, remote_version)) {
            // 当前版本已经 >= 远端版本，不需要更新
            OutputDebugStringW((L"[Update] No newer version. Current: " + Utf8ToWide(current_version) +
                L", Remote: " + Utf8ToWide(remote_version) + L"\n").c_str());
            return;
        }

        // 需要更新，获取下载链接
        std::string download_url_str = info["assets"][0]["browser_download_url"].get<std::string>();
        std::wstring download_url = Utf8ToWide(download_url_str);

        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);
        std::wstring tempFilePath = std::wstring(tempPath) + L"app_new.exe";

        if (DownloadFile(download_url, tempFilePath)) {
            ReplaceAndRestart(tempFilePath);
        } else {
            MessageBoxW(nullptr, L"下载失败，请查看调试输出", L"错误", MB_ICONERROR);
        }
    } catch (const std::exception& e) {
        std::wstring werr = Utf8ToWide(e.what());
        MessageBoxW(nullptr, (L"异常：" + werr).c_str(), L"错误", MB_ICONERROR);
    } catch (...) {
        MessageBoxW(nullptr, L"未知异常", L"错误", MB_ICONERROR);
    }
}