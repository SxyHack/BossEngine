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
	// ��ʽת��, ���ؽ��̾��
	// 
	operator HANDLE();

	// 
	// ���ؽ��̾��
	//
	HANDLE Handle();

	//
	// Modules
	//
	void AppendModule(Module* pMod);

	//
	// Threads
	// ����̵߳�Map
	// param pThread - BEThread class ptr
	//
	void AppendThread(BEThread* pThread);
	void RemoveThread(quint64 tid);
	//
	// �Ƴ����е��߳�
	//
	void RemoveThreads();
	bool ThreadIsExist(quint64 tid);
	//
	// ö�������߳�
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
