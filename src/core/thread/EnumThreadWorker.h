#pragma once

#include <QThread>
#include <QSemaphore>

class Process;

class EnumThreadWorker : public QThread
{
	Q_OBJECT

public:
	EnumThreadWorker();
	EnumThreadWorker(Process* process);
	~EnumThreadWorker();

	void SetProcess(Process* process);
	void Stop();

protected:
	void run() override;

protected:
	Process*   _Process;
	QSemaphore _ExitSE;
};
