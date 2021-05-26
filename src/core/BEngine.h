#pragma once

#include <QObject>
#include <QMutex>

#include <Windows.h>
#include <tlhelp32.h>

#include "WinExtras.h"
#include "struct/BEWorkspace.h"
#include "provider/VirtualMemoryProvider.h"

class BEngine : public QObject
{
	Q_OBJECT

public:
	~BEngine();

	static BEngine& Instance();

	//
	// 枚举所有进程, 会发出三个信号 sgEnumProcessPrepare, sgEnumProcess, sgEnumProcessDone
	// sgEnumProcessPrepare: 在循环开始前发射
	// sgEnumProcess: 每发现一个进程时发射
	// sgEnumProcessDone: 遍历循环结束后发射
	//
	void EnumProcess(const QString& filter = QString());

	//
	// 打开进程, 准备扫描或者读写
	// 
	BOOL OpenProcess(DWORD pid);
	// 
	// 枚举打开进程的所有模块, 需要先调用 OpenProcess.
	//
	BOOL EnumModules();

	DWORD GetLastErrorCode();
	QString GetLastErrorMessage();

	//////////////////////////////////////////////////////////////////////////
	// 工作空间
#pragma region WORKSPACE
	BEWorkspace* AddWorkspace(qint32 uniqueID, const QString& name=QString());
#pragma endregion


signals:
	void sgEnumProcessPrepare();
	void sgEnumProcess(qint32, PROCESSENTRY32);
	void sgEnumProcessDone(qint32);

private:
	BEngine();

	WinExtras      _WinExtras;
	DWORD          _AttachProcessID;
	HANDLE         _AttachProcessHandle;
	LIST_MODULE    _AttachProcessModules;
	DWORD          _LastErrorCode;
	QString        _LastErrorMessage;
	LIST_WORKSPACE _Workspaces;
	VirtualMemoryProvider _VM;
};

#define Engine BEngine::Instance()
