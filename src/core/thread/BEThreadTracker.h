#pragma once

#include <QThread>
#include <QSemaphore>

class BEThread;
class Process;

class BEThreadTracker : public QThread
{
	Q_OBJECT

public:
	BEThreadTracker(BEThread* pThread, Process* pProcess);
	~BEThreadTracker();

	void Stop();

protected:
	void run() override;

protected:
	BEThread*  _Thread;
	Process*   _Process;
	QSemaphore _ExitThreadSemaphore; // 退出线程信号
};
