#pragma once

#include <QThread>

class BEThread;

class BEThreadTracker : public QThread
{
	Q_OBJECT

public:
	BEThreadTracker(BEThread* pThread);
	~BEThreadTracker();

protected:
	BEThread* _Thread;
};
