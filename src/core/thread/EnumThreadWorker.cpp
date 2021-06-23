#include "EnumThreadWorker.h"
#include "Process.h"
#include "WinExtras.h"

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
}

void EnumThreadWorker::SetProcess(Process* process)
{
	_Process = process;
}

void EnumThreadWorker::Stop()
{
	requestInterruption();
	_ExitSE.tryAcquire(1, 1000L);
}

void EnumThreadWorker::run()
{
	BOOL ret = FALSE;
	THREADENTRY32 tlh32Entry = { 0 };
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
		quint64 count = 0;

		_Process->RemoveThreads();
		ret = Thread32First(hSnap, &tlh32Entry);
		do
		{
			if (tlh32Entry.th32OwnerProcessID == _Process->PID)
			{
				_Process->AppendThread(new BEThread(tlh32Entry));
				count++;
			}
		} while (ret = Thread32Next(hSnap, &tlh32Entry));

		qDebug("发现线程: %d", count);
		QThread::msleep(500);
	}

	CloseHandle(hSnap);

	_ExitSE.release();

	qDebug("Exit Thread");
}
