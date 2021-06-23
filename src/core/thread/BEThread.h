#pragma once

#include <QObject>
#include <QMap>
#include "global.h"

//
// BEThread class
//
class BEThread : public QObject
{
	Q_OBJECT

public:
	BEThread(THREADENTRY32& tlh32);
	~BEThread();

public:
	THREADENTRY32 Tlh32Entry;
	quint64       ThreadID;
};

typedef QMap<quint64, BEThread*> MAP_THREAD;
