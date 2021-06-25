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
	QSemaphore _ExitThreadSemaphore; // �˳��߳��ź�
	quint64    _LastRip;

	LIST_THREAD_FRAME _ListThreadFrame;
};
