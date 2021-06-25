#pragma once

#include <QThread>
#include <QSemaphore>

class BEThread;
class Process;

typedef QList<CONTEXT> LIST_THREAD_FRAME;

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
	quint64    _LastRip;

	LIST_THREAD_FRAME _ListThreadFrame;
};
