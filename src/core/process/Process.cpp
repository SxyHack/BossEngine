#include "Process.h"
#include "WinExtras.h"

Process::Process() : QObject(nullptr)
	, Handle(NULL)
	, PID(0)
	, _Error(0)
{
}

Process::~Process()
{
}

BOOL Process::Open(DWORD dwPID)
{
	PID = dwPID;
	Handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (Handle == NULL)
	{
		_Error = GetLastError();
		_ErrMessage = WinExtras::FormatLastError(_Error);
		qCritical("OpenProcess[%d] failed, ERROR:0x%x", dwPID, _Error);
		return FALSE;
	}

	return TRUE;
}
