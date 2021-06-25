#include "EnumModuleWorker.h"
#include "Process.h"
#include "WinExtras.h"


EnumModuleWorker::EnumModuleWorker(Process* process)
	: QThread(nullptr)
	, _Process(process)
{
}

EnumModuleWorker::~EnumModuleWorker()
{
	Stop();
}

void EnumModuleWorker::Stop()
{
	if (isInterruptionRequested()) {
		qWarning("Already stop.");
		return;
	}

	qDebug("Stopping...");
	requestInterruption();
	if (_ExitSemaphore.tryAcquire(1, 1000L))
		_ExitSemaphore.release();
	qInfo("Stopped");
}

void EnumModuleWorker::WaitForInit()
{
	_InitSemaphore.acquire();
}

void EnumModuleWorker::run()
{
	MODULEENTRY32 tlh32Entry;
	ZeroMemory(&tlh32Entry, sizeof(MODULEENTRY32));
	tlh32Entry.dwSize = sizeof(MODULEENTRY32);

	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, _Process->PID);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		DWORD dwLastError = GetLastError();
		auto message = WinExtras::FormatLastError(dwLastError);
		qWarning("CreateToolhelp32Snapshot(TH32CS_SNAPMODULE) FAIL, 0x%08X %s",
			dwLastError,
			message.toUtf8().data());
		_ExitSemaphore.release();
		return;
	}

	while (!isInterruptionRequested())
	{
		quint64 count = 0;
		BOOL ret = Module32First(hSnap, &tlh32Entry);
		do
		{
			if (tlh32Entry.th32ProcessID == _Process->PID)
			{
				QString qsModName = QString::fromWCharArray(tlh32Entry.szModule);
				if (!_Process->ModuleIsExist(qsModName))
				{
					auto pModule = Module::Create(_Process, tlh32Entry);
					_Process->AppendModule(pModule);
					count++;
				}
			}
		} while ((ret = Module32Next(hSnap, &tlh32Entry)) && !isInterruptionRequested());
		if (count > 0) 
			qDebug("模块总数: %d", count);
		_InitSemaphore.release();
		QThread::msleep(500);
	}

	CloseHandle(hSnap);
	_ExitSemaphore.release();
	qDebug("Exit EnumModule");
}
