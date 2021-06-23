#pragma once

#include <QObject>
#include "global.h"
#include "Module.h"
#include "BEThread.h"

enum class ARCH
{
	Invalid,
	Native86,
	Native64,
	Dotnet86,
	Dotnet64,
	DotnetAnyCpu,
	DotnetAnyCpuPrefer32
};

class EnumThreadWorker;


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

	//
	// Threads
	// 添加线程到Map
	// param pThread - BEThread class ptr
	//
	void AppendThread(BEThread* pThread);
	void RemoveThread(quint64 tid);
	//
	// 移除所有的线程
	//
	void RemoveThreads();
	bool ThreadIsExist(quint64 tid);
	//
	// 枚举所有线程
	//
	void EnumThreads();
	void StartThreadTrack();

public:
	static ARCH GetPEArch(const QString& qsFileName);
	static BOOL GetWindowTitle(DWORD dwPID, QString& qsWinTitle);
	static BOOL IsMainWindow(HWND hWnd);

public:
	DWORD PID;

private:
	HANDLE     _Handle;
	DWORD      _Error;
	QString    _ErrMessage;
	MAP_MOUDLE _MoudleMap;
	MAP_THREAD _ThreadMap;

	EnumThreadWorker* _EnumThread;
};
