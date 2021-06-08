#include "Debugger.h"
#include <QMutex>

QMutex mxDebug;

Debugger::Debugger()
	: QThread()
	, _dwContinueCode(DBG_CONTINUE)
{
}

Debugger::~Debugger()
{
}

Debugger& Debugger::Instance()
{
	QMutexLocker lock(&mxDebug);

	static Debugger instance;
	return instance;
}

BOOL Debugger::EnableDebugPrivilege(BOOL bEnable)
{
	// 附给本进程特权，以便访问系统进程
	BOOL bOk = FALSE;
	HANDLE hToken;

	// 打开一个进程的访问令牌
	if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		// 取得特权名称为“SetDebugPrivilege”的LUID
		LUID uID;
		::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &uID);

		// 调整特权级别
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = uID;
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		bOk = (::GetLastError() == ERROR_SUCCESS);

		// 关闭访问令牌句柄
		::CloseHandle(hToken);
	}

	return bOk;
}

void Debugger::run()
{
	while (!isInterruptionRequested())
	{
		// Fix based on work by https://github.com/number201724
		if (!WaitForDebugEvent(&_DebugEvent, 100))
		{
			continue;
		}

		switch (_DebugEvent.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:
			// Process the exception code. When handling 
			// exceptions, remember to set the continuation 
			// status parameter (dwContinueStatus). This value 
			// is used by the ContinueDebugEvent function. 

			switch (_DebugEvent.u.Exception.ExceptionRecord.ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION:
				// First chance: Pass this on to the system. 
				// Last chance: Display an appropriate error. 
				break;

			case EXCEPTION_BREAKPOINT:
				// First chance: Display the current 
				// instruction and register values. 
				break;

			case EXCEPTION_DATATYPE_MISALIGNMENT:
				// First chance: Pass this on to the system. 
				// Last chance: Display an appropriate error. 
				break;

			case EXCEPTION_SINGLE_STEP:
				// First chance: Update the display of the 
				// current instruction and register values. 
				break;

			case DBG_CONTROL_C:
				// First chance: Pass this on to the system. 
				// Last chance: Display an appropriate error. 
				break;

			default:
				// Handle other exceptions. 
				break;
			}
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			// As needed, examine or change the thread's registers 
			// with the GetThreadContext and SetThreadContext functions; 
			// and suspend and resume thread execution with the 
			// SuspendThread and ResumeThread functions. 

			//dwContinueStatus = OnCreateThreadDebugEvent(DebugEv);
			break;

		case CREATE_PROCESS_DEBUG_EVENT:
			// As needed, examine or change the registers of the
			// process's initial thread with the GetThreadContext and
			// SetThreadContext functions; read from and write to the
			// process's virtual memory with the ReadProcessMemory and
			// WriteProcessMemory functions; and suspend and resume
			// thread execution with the SuspendThread and ResumeThread
			// functions. Be sure to close the handle to the process image
			// file with CloseHandle.

			//dwContinueStatus = OnCreateProcessDebugEvent(DebugEv);
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			// Display the thread's exit code. 

			//dwContinueStatus = OnExitThreadDebugEvent(DebugEv);
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			// Display the process's exit code. 

			//dwContinueStatus = OnExitProcessDebugEvent(DebugEv);
			break;

		case LOAD_DLL_DEBUG_EVENT:
			// Read the debugging information included in the newly 
			// loaded DLL. Be sure to close the handle to the loaded DLL 
			// with CloseHandle.
			_dwContinueCode = HandleLoadDLL();

			//dwContinueStatus = OnLoadDllDebugEvent(DebugEv);
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			// Display a message that the DLL has been unloaded. 

			//dwContinueStatus = OnUnloadDllDebugEvent(DebugEv);
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			// Display the output debugging string. 

			//dwContinueStatus = OnOutputDebugStringEvent(DebugEv);
			break;

		case RIP_EVENT:
			//dwContinueStatus = OnRipEvent(DebugEv);
			break;
		}

		// Resume executing the thread that reported the debugging event. 

		::ContinueDebugEvent(_DebugEvent.dwProcessId, _DebugEvent.dwThreadId, _dwContinueCode);
	}
}

DWORD Debugger::HandleLoadDLL()
{

}
