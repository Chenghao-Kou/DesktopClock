// header.h: 标准系统包含文件的包含文件，
// 或特定于项目的包含文件
//

#pragma once
// 让程序主动请求使用新版的通用控件库
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#define NOMINMAX
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <ctime>
#include <commctrl.h>
#include <string>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <chrono>
#include <stdexcept>
#include <format>
#include <wininet.h>
#include "packages/nlohmann.json.3.12.0/build/native/include/nlohmann/json.hpp"
#include <thread>
#include <urlmon.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "urlmon.lib")