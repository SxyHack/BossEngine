#pragma once

#include "global.h"

#include <QObject>
#include <QThread>

class Process;

//
//
//
class Debugger : public QThread
{
	Q_OBJECT

public:
	~Debugger();
	static Debugger& Instance();

	//
	// brief 启动调试器
	// param process - 进程指针
	//
	void Start(Process* process);
	void Stop();

protected:
	NTSTATUS ConnectToDbg();
	//
	// brief 调试关联
	// param proc - 进程指针
	// return BOOL - 成功返回TRUE, 否则返回FALSE
	// 
	BOOL Attach();

protected:
	void run() override;
	DWORD HandleLoadDLL();

public:
	// static public
	static BOOL EnableDebugPrivilege(BOOL bEnable);

private:
	Debugger();

private:
	DWORD       _dwContinueCode;
	DEBUG_EVENT _DebugEvent;
	Process*    _Process;
	BOOL        _AttachedToProcess;
	HANDLE      _DebugObject;
};


#define BEDebugger Debugger::Instance()