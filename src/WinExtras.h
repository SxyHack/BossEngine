// header
#pragma once

#include <QObject>
#include <QList>
#include <Windows.h>
#include <tlhelp32.h>
#include <QtWin>

typedef QList<HWND>          LIST_HWND;
typedef QList<MODULEENTRY32> LIST_MODULE;

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
	QString FormatLastError(DWORD lastErr);
	// 
	// 查询进程的模块列表信息, 使用WindowsAPI
	// 参数:
	// hProcess [IN] 进程句柄
	//
	BOOL QueryProcessModules(IN HANDLE hProcess);
	//
	// 查询进程的模块列表信息, 使用ToolHelpLibrary
	// 参数:
	// dwPID - 传入进程ID
	//
	BOOL QueryProcessModulesTH(IN DWORD dwPID, OUT LIST_MODULE& modules);
private:


private:
	BOOLEAN _DebugPrivilege;
};
