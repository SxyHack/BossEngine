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
	// brief ���Թ���
	// param HANDLE hProcess - ���̵ľ��
	// return BOOL - �ɹ�����TRUE, ���򷵻�FALSE
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
