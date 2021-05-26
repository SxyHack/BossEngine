#include "WinExtras.h"
#include "undocument.h"
#include <psapi.h>

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
	NTSTATUS status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &_DebugPrivilege);
	if (!NT_SUCCESS(status)) {
		return FALSE;
	}

	return _DebugPrivilege;
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

BOOL WinExtras::QueryProcessModules(IN HANDLE hProcess)
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


BOOL WinExtras::QueryProcessModulesTH(IN DWORD dwPID, OUT LIST_MODULE& modules)
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
