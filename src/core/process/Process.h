#pragma once

#include <QObject>
#include "global.h"
#include "Module.h"

enum class PE_ARCH
{
	Invalid,
	Native86,
	Native64,
	Dotnet86,
	Dotnet64,
	DotnetAnyCpu,
	DotnetAnyCpuPrefer32
};


class Process : public QObject
{
	Q_OBJECT

public:
	Process();
	~Process();

	BOOL Open(DWORD dwPID);
	BOOL NtOpen(DWORD dwPID);
	BOOL IsOpen();

	BOOL Close();

	//
	// 隐式转换, 返回进程句柄
	// 
	operator HANDLE();

	// 
	// 返回进程句柄
	//
	HANDLE Handle();

	//
	// Modules
	//
	void AppendModule(Module* pMod);

public:
	static PE_ARCH GetPEArch(const QString& qsFileName);
	static BOOL    GetWindowTitle(DWORD dwPID, QString& qsWinTitle);
	static BOOL    IsMainWindow(HWND hWnd);
public:
	DWORD PID;

private:
	HANDLE     _Handle;
	DWORD      _Error;
	QString    _ErrMessage;
	MAP_MOUDLE _MoudleMap;
};
