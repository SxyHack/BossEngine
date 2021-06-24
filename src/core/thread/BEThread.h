#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include "global.h"

#ifndef CONTEXT_EXTENDED_REGISTERS
#define CONTEXT_EXTENDED_REGISTERS 0
#endif

class Process;
class BEThreadTracker;
//
// BEThread class
//
class BEThread : public QObject
{
	Q_OBJECT

public:
	BEThread(THREADENTRY32& tlh32, Process* pProcess);
	~BEThread();

	void StartTrack();

public:
	THREADENTRY32 Tlh32Entry;
	quint64       ThreadID;
	BOOL          bCanSuspend;  // 调用SuspendThread是否能成功
	CONTEXT       Context;

private:
	Process*         _Process;
	BEThreadTracker* _Tracker;
};

typedef QMap<quint64, BEThread*>  MAP_THREAD;
typedef QList<BEThread*>         LIST_THREAD;
