#include "EnumThreadWorker.h"
#include "Process.h"
#include "WinExtras.h"
#include "BEThreadTracker.h"

EnumThreadWorker::EnumThreadWorker(Process* process)
	: QThread(nullptr)
	, _Process(process)
{
}

EnumThreadWorker::EnumThreadWorker()
	: QThread(nullptr)
	, _Process(nullptr)
{

}

EnumThreadWorker::~EnumThreadWorker()
{
	Stop();
}

void EnumThreadWorker::SetProcess(Process* process)
{
	_Process = process;
}

void EnumThreadWorker::Stop()
{
	if (isInterruptionRequested()) {
		qWarning("Already stop.");
		return;
	}

	qDebug("Stopping...");
	requestInterruption();
	if (_ExitSE.tryAcquire(1, 1000L))
		_ExitSE.release();
	qInfo("Stopped");
}

void EnumThreadWorker::run()
{
	THREADENTRY32 tlh32Entry;
	ZeroMemory(&tlh32Entry, sizeof(THREADENTRY32));
	tlh32Entry.dwSize = sizeof(THREADENTRY32);

	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, _Process->PID);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		DWORD dwLastError = GetLastError();
		auto message = WinExtras::FormatLastError(dwLastError);
		qWarning("CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD) FAIL, 0x%08X %s",
			dwLastError,
			message.toUtf8().data());
		_ExitSE.release();
		return;
	}

	while (!isInterruptionRequested())
	{
		quint64 threadCount = 0;
		BOOL ret = Thread32First(hSnap, &tlh32Entry);
		do
		{
			if (tlh32Entry.th32OwnerProcessID == _Process->PID)
			{
				if (!_Process->ThreadIsExist(tlh32Entry.th32ThreadID))
				{
					auto pThread = new BEThread(tlh32Entry, _Process);
					//if (threadCount == 0)
					//pThread->StartTrack();

					_Process->AppendThread(pThread);
					threadCount++;
				}
			}
		} while (ret = Thread32Next(hSnap, &tlh32Entry));

		if (threadCount > 0)
			qDebug("线程总数: %d", threadCount);

		QThread::msleep(500);
	}

	CloseHandle(hSnap);
	qDebug("Exit EnumThread");
	_ExitSE.release();
}
