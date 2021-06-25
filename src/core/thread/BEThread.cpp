#include "BEThread.h"
#include "BEThreadTracker.h"

BEThread::BEThread(THREADENTRY32& tlh32, Process* pProcess)
	: QObject(nullptr)
	, Tlh32Entry(tlh32)
	, ThreadID(tlh32.th32ThreadID)
	, _Process(pProcess)
	, _Tracker(new BEThreadTracker(this, pProcess))
{
	ZeroMemory(&Context, sizeof(CONTEXT));
	Context.ContextFlags = CONTEXT_ALL | CONTEXT_FLOATING_POINT | CONTEXT_EXTENDED_REGISTERS;
}

BEThread::~BEThread()
{
	_Tracker->Stop();

	delete _Tracker;
}

void BEThread::StartTrack()
{
	if (_Tracker->isRunning())
		return;

	_Tracker->start(QThread::HighPriority);
	qDebug("启动线程[%d]...", Tlh32Entry.th32ThreadID);
}