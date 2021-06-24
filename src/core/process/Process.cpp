#include "Process.h"
#include "WinExtras.h"
#include "libs/ntdll/ntdll.h"
#include "EnumThreadWorker.h"
#include "BEThreadTracker.h"

#include <QFile>
#include <QMutex>

#define MAX_FILE_PATH_SIZE 1024

static QMutex mxAppendThread;
static QMutex mxAppendMod;

Process::Process() : QObject(nullptr)
, PID(0)
, _Handle(NULL)
, _Error(0)
, _EnumThread(new EnumThreadWorker(this))
{
}

Process::~Process()
{
}

BOOL Process::Open(DWORD dwPID)
{
	PID = dwPID;
	_Handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (_Handle == NULL)
	{
		_Error = GetLastError();
		_ErrMessage = WinExtras::FormatLastError(_Error);
		qCritical("OpenProcess[%d] failed, ERROR:0x%x", dwPID, _Error);
		return FALSE;
	}

	return TRUE;
}

BOOL Process::NtOpen(DWORD dwPID)
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES objectAttributes;
	CLIENT_ID clientId;

	PID = dwPID;
	/* If we don't have a PID, look it up */
	//if (dwProcessId == MAXDWORD) dwProcessId = (DWORD_PTR)CsrGetProcessId();

	/* Open a handle to the process */
	clientId.UniqueThread = NULL;
	clientId.UniqueProcess = UlongToHandle(dwPID);
	InitializeObjectAttributes(&objectAttributes, NULL, 0, NULL, NULL);
	status = NtOpenProcess(&_Handle,
		PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE | PROCESS_VM_READ |
		PROCESS_SUSPEND_RESUME | PROCESS_QUERY_INFORMATION,
		&objectAttributes,
		&clientId);
	if (!NT_SUCCESS(status))
	{
		/* Fail */
		WinExtras::SetNtLastError(status);
		return FALSE;
	}

	/* Return the handle */
	return TRUE;
}

BOOL Process::IsOpen()
{
	return _Handle != NULL;
}

BOOL Process::Close()
{
	if (_EnumThread != nullptr)
	{
		delete _EnumThread;
		_EnumThread = nullptr;
	}

	// 关闭所有线程监控
	for (auto pThread : _ThreadMap)
	{
		delete pThread;
	}

	_ThreadMap.clear();
	NtClose(_Handle);

	qDebug("Process[%d] closed", PID);
	return TRUE;
}

BOOL Process::IsWow64()
{
	BOOL bIsWow64 = FALSE;
	if (!IsWow64Process(_Handle, &bIsWow64))
		return FALSE;

	return bIsWow64;
}

Process::operator HANDLE()
{
	return _Handle;
}

HANDLE Process::Handle()
{
	return _Handle;
}

void Process::AppendModule(Module* pMod)
{
	QMutexLocker lock(&mxAppendMod);

	_MoudleMap.insert(QRange(pMod->ModBase, pMod->ModBase + pMod->ModSize - 1), pMod);
}

void Process::AppendThread(BEThread* pThread)
{
	QMutexLocker lock(&mxAppendThread);

	if (_ThreadMap.contains(pThread->ThreadID))
		return;

	_ThreadMap.insert(pThread->ThreadID, pThread);
	_ThreadIDs.append(pThread->ThreadID);
}

void Process::RemoveThread(quint64 tid)
{
	QMutexLocker lock(&mxAppendThread);

	if (_ThreadMap.contains(tid))
	{
		_ThreadIDs.removeOne(tid);
		auto pThread = _ThreadMap.take(tid);
		delete pThread;
	}
}

void Process::RemoveThreads()
{
	QMutexLocker lock(&mxAppendThread);

	for (auto pThread : _ThreadMap)
	{
		delete pThread;
	}

	_ThreadMap.clear();
	_ThreadIDs.clear();
}

bool Process::ThreadIsExist(quint64 tid)
{
	QMutexLocker lock(&mxAppendThread);
	return _ThreadMap.contains(tid);
}

void Process::EnumThreads()
{
	if (_EnumThread->isRunning())
		return;

	_EnumThread->start(QThread::HighPriority);
}

ARCH Process::GetPEArch(const QString& qsFileName)
{
	auto result = ARCH::Invalid;
	if (!QFile::exists(qsFileName))
	{
		return ARCH::Invalid;
	}

	QFile qfile(qsFileName);
	if (!qfile.open(QIODevice::ReadOnly))
	{
		return ARCH::Invalid;
	}

	auto pFileMap = qfile.map(0, 0);
	if (pFileMap == nullptr)
	{
		return ARCH::Invalid;
	}

	try
	{
		PIMAGE_DOS_HEADER pImageDosHeader = (PIMAGE_DOS_HEADER)pFileMap;
		if (pImageDosHeader->e_magic == IMAGE_DOS_SIGNATURE)
		{
			PIMAGE_NT_HEADERS pImageNtHeader = (PIMAGE_NT_HEADERS)(ULONG_PTR(pFileMap) + pImageDosHeader->e_lfanew);
			if (pImageNtHeader->Signature == IMAGE_NT_SIGNATURE)
			{
				auto machine = pImageNtHeader->FileHeader.Machine;
				if (machine == IMAGE_FILE_MACHINE_I386 || machine == IMAGE_FILE_MACHINE_AMD64)
				{
					bool bIsDLL = (pImageNtHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL;
					bool bIsFile86 = (machine == IMAGE_FILE_MACHINE_I386);

					// Set the native architecture of the PE file (to still have something to show for if the COM directory is invalid).
					result = bIsFile86 ? ARCH::Native86 : ARCH::Native64;

					ULONG_PTR comAddr = 0, comSize = 0;
					if (bIsFile86) // x86
					{
						auto pnth32 = PIMAGE_NT_HEADERS32(pImageNtHeader);
						comAddr = pnth32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress;
						comSize = pnth32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size;
					}
					else
					{
						auto pnth64 = PIMAGE_NT_HEADERS64(pImageNtHeader);
						comAddr = pnth64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress;
						comSize = pnth64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size;
					}

					// Check if the file has a (valid) COM (.NET) directory.
					if (comAddr && comSize >= sizeof(IMAGE_COR20_HEADER))
					{
						// Find out which flavor of dotnet we're dealing with. Specifically,
						// "Any CPU" can be compiled with two flavors, "Prefer 32 bit" or not.
						// Without the 32bit preferred flag, the loader will load the .NET
						// environment based on the current platforms bitness (x86 or x64)
						// Class libraries (DLLs) cannot specify the "Prefer 32 bit".
						// https://mega.nz/#!vx5nVILR!jLafWGWhhsC0Qo5fE-3oEIc-uHBcRpraOo8L_KlUeXI
						// Binaries that do not have COMIMAGE_FLAGS_ILONLY appear to be executed
						// in a process that matches their native type.
						// https://github.com/x64dbg/x64dbg/issues/1758
						auto pcorh = PIMAGE_COR20_HEADER(ULONG_PTR(pFileMap) + comAddr);
						if (pcorh->cb == sizeof(IMAGE_COR20_HEADER))
						{
							auto flags = pcorh->Flags;
#define test(x) (flags & x) == x
#define MY_COMIMAGE_FLAGS_32BITPREFERRED 0x00020000
							if (bIsFile86) // x86
							{
								if (test(COMIMAGE_FLAGS_32BITREQUIRED))
								{
									result = !bIsDLL && test(MY_COMIMAGE_FLAGS_32BITPREFERRED) ? ARCH::DotnetAnyCpuPrefer32 : ARCH::Dotnet86;
								}
								else if (test(COMIMAGE_FLAGS_ILONLY))
								{
									result = ARCH::DotnetAnyCpu;
								}
								else
								{
									result = ARCH::Dotnet86;
								}
							}
							else // x64
							{
								result = ARCH::Dotnet64;
							}
#undef MY_COMIMAGE_FLAGS_32BITPREFERRED
#undef test
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		qWarning("EXCEPTION_EXECUTE_HANDLER");
	}

	qfile.unmap(pFileMap);
	qfile.close();

	return result;
}

static HANDLE g_hProc = NULL;
static HWND   g_hForegroundWnd = NULL;
BOOL CALLBACK cbEnumWindows(HWND hWnd, LPARAM lParam)
{
	DWORD pid = (DWORD)lParam;
	DWORD hWndPID = 0;
	GetWindowThreadProcessId(hWnd, &hWndPID);
	if (hWndPID == pid)
	{
		if (g_hForegroundWnd == NULL)  // get the foreground if no owner visible
			g_hForegroundWnd = hWnd;

		if (Process::IsMainWindow(hWnd))
		{
			g_hProc = hWnd;
			return FALSE;
		}
	}

	return TRUE;
}

BOOL Process::GetWindowTitle(DWORD dwPID, QString& qsWinTitle)
{
	EnumWindows(cbEnumWindows, dwPID);

	if (g_hProc == NULL && g_hForegroundWnd == NULL)
		return FALSE;

	TCHAR szWinTitle[MAX_FILE_PATH_SIZE];
	ZeroMemory(szWinTitle, MAX_FILE_PATH_SIZE);

	if (g_hProc) // get info from the "main window" (GW_OWNER + visible)
	{
		// go for the class name if none of the above
		if (!GetWindowText((HWND)g_hProc, szWinTitle, MAX_FILE_PATH_SIZE))
			GetClassName((HWND)g_hProc, szWinTitle, MAX_FILE_PATH_SIZE);
	}
	else if (g_hForegroundWnd) // get info from the foreground window
	{
		// go for the class name if none of the above
		if (!GetWindowTextW(g_hForegroundWnd, szWinTitle, MAX_FILE_PATH_SIZE))
			GetClassNameW(g_hForegroundWnd, szWinTitle, MAX_FILE_PATH_SIZE);
	}

	if (szWinTitle[MAX_FILE_PATH_SIZE - 1] != 0)
	{
		//protect the last surrogate of UTF-16 surrogate pair
		if (szWinTitle[MAX_FILE_PATH_SIZE - 4] < 0xDC00 || szWinTitle[MAX_FILE_PATH_SIZE - 4] > 0xDFFF)
			szWinTitle[MAX_FILE_PATH_SIZE - 4] = L'.';
		szWinTitle[MAX_FILE_PATH_SIZE - 3] = L'.';
		szWinTitle[MAX_FILE_PATH_SIZE - 2] = L'.';
		szWinTitle[MAX_FILE_PATH_SIZE - 1] = 0;
	}

	qsWinTitle = QString::fromWCharArray(szWinTitle, MAX_FILE_PATH_SIZE);

	return TRUE;
}

BOOL Process::IsMainWindow(HWND hWnd)
{
	return !GetWindow(hWnd, GW_OWNER) && IsWindowVisible(hWnd);
}
