// header
#pragma once

#include <QObject>
#include <QList>
#include <Windows.h>
#include <tlhelp32.h>
#include <QtWin>

typedef QList<HWND>          LIST_HWND;
typedef QList<MODULEENTRY32> LIST_MODULE;
typedef QList<MEMORY_BASIC_INFORMATION> LIST_MEMORY;

//
// Windows API 封装
//
class WinExtras : public QObject
{
	Q_OBJECT

public:
	WinExtras();
	~WinExtras();

	// 
	// 提升权限
	// 
	BOOL AdjustPrivilege();
	BOOL EnableDebugPrivilege(BOOL bEnable = TRUE);
	//
	// 获取窗口的句柄
	// dwPID: 进程ID
	// 返回值: 如果是NULL, 说明失败
	//
	HWND QueryWindowHandle(DWORD dwPID);
	//
	// 获取进程的应用图标
	//
	BOOL GetProcessICON(DWORD dwTargetPID, QPixmap& pixmap);
	//
	// GetLastError返回值转成字符串
	//
	static QString FormatLastError(DWORD lastErr);
	static QString FormatMemoryProtection(DWORD value);
	static QString FormatMemoryState(DWORD state);
	static QString FormatMemoryType(DWORD value);
	// 
	// 查询进程的模块列表信息, 使用WindowsAPI
	// 参数:
	// hProcess [IN] 进程句柄
	//
	BOOL EnumProcessModulesNT(IN HANDLE hProcess);
	//
	// 查询进程的模块列表信息, 使用ToolHelpLibrary
	// 参数:
	// dwPID - 传入进程ID
	//
	BOOL EnumProcessModulesTH(IN DWORD dwPID, OUT LIST_MODULE& modules);
	//
	// 枚举进程的内存信息, 用来测试一波先
	// IN hProcess: 进程句柄
	// IN lpAddress: 开始地址
	// OUT results: 返回所有的内存信息
	// 
	BOOL QueryProcessMemory(HANDLE hProcess, ULONG_PTR lpAddress, LIST_MEMORY& results);

	//
	// 获取系统目录, C:\Windows\System
	//
	QString GetSystemDir();
private:


private:
	BOOLEAN _DebugPrivilege;
};
