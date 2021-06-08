#pragma once

#include <QObject>
#include "global.h"

class Process : public QObject
{
	Q_OBJECT

public:
	Process();
	~Process();

	BOOL Open(DWORD dwPID);

public:
	HANDLE Handle;
	DWORD  PID;

private:
	DWORD   _Error_;
	QString _ErrMessage_;
};
