#include "BEThreadTracker.h"
#include "WinExtras.h"

BEThreadTracker::BEThreadTracker(BEThread* pThread, Process* pProcess)
	: QThread(nullptr)
	, _Thread(pThread)
	, _Process(pProcess)
{
}

BEThreadTracker::~BEThreadTracker()
{
}

void BEThreadTracker::Stop()
{
	if (isRunning())
	{
		requestInterruption();
		_ExitThreadSemaphore.tryAcquire(1, 1000);
	}
}

void BEThreadTracker::run()
{
	while (!isInterruptionRequested())
	{
		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, _Thread->ThreadID);
		if (hThread == NULL)
		{
			auto dwError = GetLastError();
			auto qsMessage = WinExtras::FormatLastError(dwError);
			qWarning("OpenThread Failed. %s(%d)", qsMessage.toUtf8().data(), dwError);
			break;
		}

		if (SuspendThread(hThread) == -1)
		{
			CloseHandle(hThread);
			_Thread->bCanSuspend = FALSE;
			auto dwError = GetLastError();
			auto qsMessage = WinExtras::FormatLastError(dwError);
			qCritical("SuspendThread(%d) Failed. %s(%d)", _Thread->ThreadID, qsMessage.toUtf8().data(), dwError);
			continue;
		}

		if (!GetThreadContext(hThread, &_Thread->Context))
		{
			ResumeThread(hThread);
			CloseHandle(hThread);
			auto dwError = GetLastError();
			auto qsMessage = WinExtras::FormatLastError(dwError);
			qCritical("GetThreadContext Failed. %s(%d)", qsMessage.toUtf8().data(), dwError);
			continue;
		}

		//qDebug("线程[%d]RIP:%X", _Thread->ThreadID, _Thread->Context.Rip);

		ResumeThread(hThread);
		CloseHandle(hThread);
	}

	_ExitThreadSemaphore.release();
	qDebug("跟踪线程[%d]结束.", _Thread->ThreadID);
}
