#pragma once

#include <QObject>
#include <QMutex>

#include <Windows.h>
#include <tlhelp32.h>

#include "WinExtras.h"

#include "core/model/BEWorkspace.h"
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
	HANDLE GetProcessHandle();
	//////////////////////////////////////////////////////////////////////////
	// 
	// 枚举打开进程的所有模块, 需要先调用 OpenProcess.
	//
	BOOL EnumModules();
	LIST_MODULE& GetModules();
	//
	// 将模块添加到_IncludeModules数组, 用来在搜索内容线程中使用
	//
	void IncludeModule(const MODULEENTRY32& mod);
	//
	// 清空 _IncludeModules 数组
	//
	void RemoveIncludeModules();
	LIST_MODULE& GetIncludeModules();
	//////////////////////////////////////////////////////////////////////////
	//
	// 获取所有模块的总字节数
	//
	DWORD64 GetModulesSize();
	//
	// 枚举进程的内存块
	//
	BOOL EnumVirtualMemory();

	//
	// 获取GetLastError返回值
	//
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
	LIST_MODULE    _IncludeModules;
	DWORD          _LastErrorCode;
	QString        _LastErrorMessage;
	LIST_WORKSPACE _Workspaces;
	VirtualMemoryProvider _VM;
};

#define Engine BEngine::Instance()
