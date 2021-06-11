#include "WinExtras.h"
#include "libs/ntdll/ntdll.h"
#include "global.h"

#include <psapi.h>
#include <QtDebug>

WinExtras::WinExtras()
	: QObject(nullptr)
	, _DebugPrivilege(FALSE)
{
}

WinExtras::~WinExtras()
{
}

BOOL WinExtras::AdjustPrivilege()
{
	//NTSTATUS status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &_DebugPrivilege);
	//if (!NT_SUCCESS(status)) {
	//	return FALSE;
	//}

	//return _DebugPrivilege;
	return FALSE;
}

BOOL WinExtras::EnableDebugPrivilege(BOOL bEnable)
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

HWND WinExtras::QueryWindowHandle(DWORD dwTargetPID)
{
	DWORD dwPID = 0;
	HWND hTargetWnd = NULL;
	HWND hWnd = ::GetTopWindow(NULL);

	while (hWnd)
	{
		DWORD dwThreadID = ::GetWindowThreadProcessId(hWnd, &dwPID);
		if (dwThreadID == 0)
			continue;

		if (dwPID == dwTargetPID)
		{
			hTargetWnd = hWnd;
			break;
		}

		hWnd = ::GetNextWindow(hWnd, GW_HWNDNEXT);
	}

	if (hTargetWnd != NULL)
	{
		HWND hParent = NULL;
		// 循环查找父窗口，以便保证返回的句柄是最顶层的窗口句柄
		while ((hParent = ::GetParent(hTargetWnd)) != NULL)
		{
			hTargetWnd = hParent;
		}
	}

	return hTargetWnd;
}

BOOL WinExtras::GetProcessICON(DWORD dwTargetPID, QPixmap& pixmap)
{
	HICON hIcon = NULL;
	HWND hWnd = QueryWindowHandle(dwTargetPID);
	if (hWnd == NULL) {
		return FALSE;
	}

	LRESULT ret = ::SendMessageTimeout(hWnd, WM_GETICON, 0, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&hIcon);
	if (!ret)
	{
		DWORD dwlastError = GetLastError();
		QString lastMessage = FormatLastError(dwlastError);
		qCritical("SendMessageTimeout Failed, Code:%x, Msg:%s", dwlastError, lastMessage.toUtf8().data());
		return FALSE;
	}

	if (hIcon == NULL) {
		hIcon = (HICON)::GetClassLongPtr(hWnd, GCLP_HICONSM);
	}

	if (hIcon == NULL) {
		return FALSE;
	}

	pixmap = QtWin::fromHICON(hIcon);

	return TRUE;
}

QString WinExtras::FormatLastError(DWORD lastErr)
{
	LPTSTR lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		lastErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

#ifdef UNICODE
	return QString::fromWCharArray(lpMsgBuf);
#else
	return QString::fromLocal8Bit(lpMsgBuf);
#endif
}

DWORD WinExtras::SetNtLastError(NTSTATUS status)
{
	DWORD dwErrCode = RtlNtStatusToDosError(status);
	SetLastError(dwErrCode);
	return dwErrCode;
}

QString WinExtras::FormatMemoryProtection(DWORD value)
{
	if (value == 0)
		return "PAGE_CALLER_NOT_ACCESS(0)";

	QString qsFormat;

	if (value & PAGE_EXECUTE)
		qsFormat += "PAGE_EXECUTE|";
	if (value & PAGE_EXECUTE_READ)
		qsFormat += "PAGE_EXECUTE_READ|";
	if (value & PAGE_EXECUTE_READWRITE)
		qsFormat += "PAGE_EXECUTE_READWRITE|";
	if (value & PAGE_EXECUTE_WRITECOPY)
		qsFormat += "PAGE_EXECUTE_WRITECOPY|";
	if (value & PAGE_NOACCESS)
		qsFormat += "PAGE_NOACCESS|";
	if (value & PAGE_READONLY)
		qsFormat += "PAGE_READONLY|";
	if (value & PAGE_READWRITE)
		qsFormat += "PAGE_READWRITE|";
	if (value & PAGE_WRITECOPY)
		qsFormat += "PAGE_WRITECOPY|";
	if (value & PAGE_TARGETS_INVALID)
		qsFormat += "PAGE_TARGETS_INVALID|";
	if (value & PAGE_GUARD)
		qsFormat += "PAGE_GUARD|";
	if (value & PAGE_NOCACHE)
		qsFormat += "PAGE_NOCACHE|";
	if (value & PAGE_WRITECOMBINE)
		qsFormat += "PAGE_WRITECOMBINE|";

	if (qsFormat.isEmpty())
		qsFormat = QString("PAGE_PROTECT_ERROR(0x%1)").arg(value, 0, 16);
	else
		qsFormat = qsFormat.left(qsFormat.length() - 1); // 删除最后一个'|'

	return qsFormat;
}

QString WinExtras::FormatMemoryState(DWORD state)
{
	QString qsFormat;
	if (state & MEM_COMMIT)
		qsFormat += "MEM_COMMIT|";
	if (state & MEM_FREE)
		qsFormat += "MEM_FREE|";
	if (state & MEM_RESERVE)
		qsFormat += "MEM_RESERVE|";

	if (qsFormat.isEmpty())
		qsFormat = QString("MEM_STATE_ERROR(0x%1)").arg(state, 0, 16);
	else
		qsFormat = qsFormat.left(qsFormat.length() - 1); // 删除最后一个'|'

	return qsFormat;
	//switch (state)
	//{
	//case 0x1000: return "MEM_COMMIT";
	//case 0x10000: return "MEM_FREE";
	//case 0x2000: return "MEM_RESERVE";
	//default: return QString("MEM_STATE_ERROR(0x%1)").arg(state, 0, 16);
	//}
}

QString WinExtras::FormatMemoryType(DWORD value)
{
	QString qsFormat;

	if (value & MEM_IMAGE)
		qsFormat += "MEM_IMAGE|";
	if (value & MEM_MAPPED)
		qsFormat += "MEM_MAPPED|";
	if (value & MEM_PRIVATE)
		qsFormat += "MEM_PRIVATE|";

	if (qsFormat.isEmpty())
		qsFormat = QString("MEM_TYPE_ERROR(0x%1)").arg(value, 0, 16);
	else
		qsFormat = qsFormat.left(qsFormat.length() - 1); // 删除最后一个'|'

	return qsFormat;
}

BOOL WinExtras::EnumProcessModulesNT(IN HANDLE hProcess)
{
	DWORD cbBytesNeeded = 0;
	EnumProcessModulesEx(hProcess, NULL, 0, &cbBytesNeeded, LIST_MODULES_ALL);

	DWORD cb = cbBytesNeeded + 0x1000;
	HMODULE* lpModuleEntry = (HMODULE*)malloc(cb);
	if (lpModuleEntry == NULL)
	{
		qCritical("malloc error");
		return FALSE;
	}

	EnumProcessModulesEx(hProcess, lpModuleEntry, cb, &cbBytesNeeded, LIST_MODULES_ALL);

	DWORD dwLength = cbBytesNeeded / sizeof(HMODULE);
	MODULEINFO moduleInfo;

	for (int i = 0; i < dwLength; i++)
	{
		HMODULE hMod = lpModuleEntry[i];
		DWORD cbModule = sizeof(MODULEINFO);
		GetModuleInformation(hProcess, hMod, &moduleInfo, cbModule);

		TCHAR szModFile[1024];
		GetModuleFileName(hMod, szModFile, 1024);
		qDebug("Mod: %c Base:0x%08x Size:%d Entry:0x%08x", szModFile, 
			moduleInfo.lpBaseOfDll, 
			moduleInfo.SizeOfImage, 
			moduleInfo.EntryPoint);
	}

	return TRUE;
}


BOOL WinExtras::EnumProcessModulesTH(IN DWORD dwPID, OUT LIST_MODULE& modules)
{
	BOOL ret = FALSE;
	MODULEENTRY32 moduleEntry = { 0 };
	moduleEntry.dwSize = sizeof(MODULEENTRY32);

	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		DWORD dwLastError = GetLastError();
		auto message = FormatLastError(dwLastError);
		qWarning("CreateToolhelp32Snapshot FAIL, 0x%08X %s", dwLastError, message.toUtf8().data());
		return FALSE;
	}

	ret = Module32First(hSnap, &moduleEntry);
	do 
	{
		modules.append(moduleEntry);
	} while (ret = Module32Next(hSnap, &moduleEntry));

	CloseHandle(hSnap);

	return !modules.isEmpty();
}

BOOL WinExtras::QueryProcessMemory(HANDLE hProcess, ULONG_PTR lpAddress, LIST_MEMORY& results)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	ULONG_PTR ulQueryAddr = lpAddress;

	while (::VirtualQueryEx(hProcess, (LPCVOID)ulQueryAddr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		ulQueryAddr += mbi.RegionSize;
		auto qsAllocMemProtect = FormatMemoryProtection(mbi.AllocationProtect);
		auto qsMemProtect = FormatMemoryProtection(mbi.Protect);
		auto qsMemState = FormatMemoryState(mbi.State);
		auto qsMemType = FormatMemoryType(mbi.Type);

		TCHAR szModName[MAX_PATH];
		auto size = sizeof(szModName) / sizeof(TCHAR);
		::GetModuleFileNameEx(hProcess, (HMODULE)mbi.AllocationBase, szModName, size);

		qDebug("VM(%d): %p %p %u %s %s %s %s", 
			mbi.PartitionId, 
			mbi.BaseAddress, 
			mbi.AllocationBase,
			mbi.RegionSize,
			qsAllocMemProtect.toUtf8().data(),
			qsMemProtect.toUtf8().data(), 
			qsMemState.toUtf8().data(), 
			qsMemType.toUtf8().data()
		);

		qDebug() << QString::fromWCharArray(szModName);
	}

	return TRUE;
}

QString WinExtras::GetSystemDir()
{
	TCHAR szPath[512] = { 0 };
	UINT nSize = GetSystemDirectory(szPath, 512);
	return QString::fromWCharArray(szPath, nSize);
}

QString WinExtras::TranslateNativeName(TCHAR* szName)
{
	TCHAR szTranslatedName[MAX_FILE_PATH_SIZE] = { 0 };
	TCHAR szDeviceName[3] = L"A:";
	TCHAR szDeviceCOMName[5] = L"COM0";
	int CurrentDeviceLen;

	while (szDeviceName[0] <= 0x5A)
	{
		RtlZeroMemory(szTranslatedName, MAX_FILE_PATH_SIZE);
		if (QueryDosDevice(szDeviceName, szTranslatedName, MAX_PATH * 2) > NULL)
		{
			CurrentDeviceLen = lstrlen(szTranslatedName);
			lstrcat(szTranslatedName, (LPTSTR)(szName + CurrentDeviceLen));
			if (lstrcmpi(szTranslatedName, szName) == NULL)
			{
				RtlZeroMemory(szTranslatedName, MAX_FILE_PATH_SIZE);
				lstrcat(szTranslatedName, szDeviceName);
				lstrcat(szTranslatedName, (LPTSTR)(szName + CurrentDeviceLen));
				return QString::fromWCharArray(szTranslatedName);
			}
		}
		szDeviceName[0]++;
	}

	while (szDeviceCOMName[3] <= 0x39)
	{
		RtlZeroMemory(szTranslatedName, MAX_FILE_PATH_SIZE);
		if (QueryDosDevice(szDeviceCOMName, szTranslatedName, MAX_PATH * 2) > NULL)
		{
			CurrentDeviceLen = lstrlen(szTranslatedName);
			lstrcat(szTranslatedName, szName + CurrentDeviceLen);
			if (lstrcmpi(szTranslatedName, szName) == NULL)
			{
				RtlZeroMemory(szTranslatedName, MAX_FILE_PATH_SIZE);
				lstrcat(szTranslatedName, szDeviceCOMName);
				lstrcat(szTranslatedName, szName + CurrentDeviceLen);
				return QString::fromWCharArray(szTranslatedName);
			}
		}
		szDeviceCOMName[3]++;
	}

	return QString();
}

//BOOL WinExtras::QueryProcessInformation(HANDLE hProcess)
//{
//	PPEB lpPEB;
//	PROCESS_BASIC_INFORMATION pbi;
//	ULONG ulPEBSize = sizeof(PROCESS_BASIC_INFORMATION);
//	ULONG ulReturnSize = 0;
//
//	NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, ulPEBSize, &ulReturnSize);
//	if (!NT_SUCCESS(status))
//	{
//		return FALSE;
//	}
//
//	return TRUE;
//}
