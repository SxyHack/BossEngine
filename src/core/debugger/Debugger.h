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
	// brief ����������
	// param process - ����ָ��
	//
	void Start(Process* process);
	void Stop();

protected:
	NTSTATUS ConnectToDbg();
	//
	// brief ���Թ���
	// param proc - ����ָ��
	// return BOOL - �ɹ�����TRUE, ���򷵻�FALSE
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