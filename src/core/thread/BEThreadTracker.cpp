#include "BEThreadTracker.h"
#include "WinExtras.h"

BEThreadTracker::BEThreadTracker(BEThread* pThread, Process* pProcess)
	: QThread(nullptr)
	, _Thread(pThread)
	, _Process(pProcess)
	, _ExitThreadSemaphore(1)
	, _LastRip(0)
{
}

BEThreadTracker::~BEThreadTracker()
{
	Stop();
}

void BEThreadTracker::Stop()
{
	if (isInterruptionRequested()) {
		qWarning("Already stop.");
		return;
	}

	qDebug("Thread-[%d] stopping...", _Thread->ThreadID);
	requestInterruption();

	if (_ExitThreadSemaphore.tryAcquire(1, 1000L))
		_ExitThreadSemaphore.release();
	qInfo("Stopped");
}

void BEThreadTracker::run()
{
	_ExitThreadSemaphore.acquire();
	HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, _Thread->ThreadID);
	if (hThread == NULL)
	{
		auto dwError = GetLastError();
		auto qsMessage = WinExtras::FormatLastError(dwError);
		qWarning("OpenThread Failed. %s(%d)", qsMessage.toUtf8().data(), dwError);
		goto THREAD_END;
	}

	while (!isInterruptionRequested())
	{
		//if (SuspendThread(hThread) == -1)
		//{
		//	CloseHandle(hThread);
		//	_Thread->bCanSuspend = FALSE;
		//	auto dwError = GetLastError();
		//	auto qsMessage = WinExtras::FormatLastError(dwError);
		//	qCritical("SuspendThread(%d) Failed. %s(%d)", _Thread->ThreadID, qsMessage.toUtf8().data(), dwError);
		//	continue;
		//}

		CONTEXT context;
		context.ContextFlags = CONTEXT_ALL | CONTEXT_FLOATING_POINT;
		if (!GetThreadContext(hThread, &context))
		{
			//ResumeThread(hThread);
			continue;
		}

		if (_LastRip != context.Rip)
		{
			_LastRip = context.Rip;
			_ListThreadFrame.append(context);
			qDebug("线程[%d] RIP:%X", _Thread->ThreadID, _LastRip);
		}

		QThread::msleep(1);
		//ResumeThread(hThread);
		//CloseHandle(hThread);
	}

	CloseHandle(hThread);

THREAD_END:
	qDebug("跟踪线程[%d]结束.", _Thread->ThreadID);
	_ExitThreadSemaphore.release();
}
