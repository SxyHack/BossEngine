#pragma once

#include "global.h"

#include <QObject>
#include <QThread>

class Debugger : public QThread
{
	Q_OBJECT

public:
	~Debugger();
	static Debugger& Instance();

	//
	// brief 调试关联
	// param HANDLE hProcess - 进程的句柄
	// return BOOL - 成功返回TRUE, 否则返回FALSE
	// 
	BOOL Attach(HANDLE hProcess);

protected:


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
};
