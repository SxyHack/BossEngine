#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include "global.h"

#ifndef CONTEXT_EXTENDED_REGISTERS
#define CONTEXT_EXTENDED_REGISTERS 0
#endif

typedef QMap<quint64, quint64> MAP_RIP;
typedef QList<quint64>         LIST_RIP;

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
	CONTEXT       Context;
	MAP_RIP       MapRip;

private:
	Process*         _Process;
	BEThreadTracker* _Tracker;
};

typedef QMap<quint64, BEThread*>  MAP_THREAD;
typedef QList<BEThread*>         LIST_THREAD;
