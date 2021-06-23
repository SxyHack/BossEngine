#include "BEngine.h"
#include "undocument.h"
#include <QFileInfo>

Q_DECLARE_METATYPE(PROCESSENTRY32);
Q_DECLARE_METATYPE(MODULEENTRY32);

BEngine::BEngine() : QObject(nullptr)
	, _AttachProcessID(0)
	, _AttachProcessHandle(NULL)
	, _LastErrorCode(0)
{
}

BEngine::~BEngine()
{
}

BEngine& BEngine::Instance()
{
	static QMutex mutex;
	QMutexLocker lock(&mutex);

	static BEngine instance;
	return instance;
}

void BEngine::ListProcesses(const QString& filter)
{
	BOOL ret = FALSE;
	DWORD dwRow = 0;
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		qWarning("CreateToolhelp32Snapshot FAIL, %0x08X", GetLastError());
		return;
	}

	emit sgListProcessStart();

	ret = ::Process32First(hSnap, &pe32);
	do 
	{
		// System process and Idle process have special PID.
		if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == 4)
			continue;

		// 过滤自己
		if (pe32.th32ProcessID == GetCurrentProcessId())
			continue;

		QString processName = QString::fromWCharArray(pe32.szExeFile);

		if (filter.isEmpty() || processName.contains(filter)) {
			emit sgListProcess(dwRow++, pe32);
		}
	} while (ret = ::Process32Next(hSnap, &pe32));

	emit sgListProcessDone(dwRow);
	CloseHandle(hSnap);
}


BOOL BEngine::OpenProcess(DWORD pid)
{
	if (!_AttachProcess.NtOpen(pid))
		return FALSE;

	//BEDebugger.Start(&_AttachProcess);
	//_AttachProcess.EnumThreads();

	return TRUE;
	//_AttachProcessID = pid;
	//_AttachProcessHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	//if (_AttachProcessHandle == NULL)
	//{
	//	_LastErrorCode = GetLastError();
	//	_LastErrorMessage = _WinExtras.FormatLastError(_LastErrorCode);
	//	qCritical("OpenProcess[%d] failed, ERROR:0x%x", pid, _LastErrorCode);
	//	return FALSE;
	//}
	//return EnumModules();
}

HANDLE BEngine::GetProcessHandle()
{
	return _AttachProcessHandle;
}

BOOL BEngine::EnumModules()
{
	_AttachProcessModules.clear();
	// 查询模块
	if (!_WinExtras.EnumProcessModulesTH(_AttachProcessID, _AttachProcessModules))
	{
		qCritical("QueryProcessModulesTH Failed");
		return FALSE;
	}

	return TRUE;
}

LIST_MODULE& BEngine::GetModules()
{
	return _AttachProcessModules;
}

void BEngine::IncludeModule(const MODULEENTRY32& mod)
{
	_IncludeModules.append(mod);
}

void BEngine::RemoveIncludeModules()
{
	_IncludeModules.clear();
}

LIST_MODULE& BEngine::GetIncludeModules()
{
	return _IncludeModules;
}

qint64 BEngine::QueryStaticAddress(quint64 ulAddress, QString& modName)
{
	for (auto& mod : _AttachProcessModules)
	{
		quint64 dwBegAddr = (quint64)mod.modBaseAddr;
		quint64 dwEndAddr = dwBegAddr + mod.modBaseSize;

		if (dwBegAddr < ulAddress && ulAddress < dwEndAddr)
		{
			modName = QString::fromWCharArray(mod.szModule);
			return  ulAddress - dwBegAddr;
		}
	}

	return -1;
}

DWORD64 BEngine::GetModulesSize()
{
	DWORD64 dwSize = 0;

	for (auto& mod : _AttachProcessModules)
	{
		dwSize += mod.modBaseSize;
	}

	return dwSize;
}

BOOL BEngine::EnumVirtualMemory()
{
	return FALSE;
}

DWORD BEngine::GetLastErrorCode()
{
	return _LastErrorCode;
}

QString BEngine::GetLastErrorMessage()
{
	return _LastErrorMessage;
}

BEWorkspace* BEngine::AddWorkspace(qint32 uniqueID, const QString& name)
{
	BEWorkspace* workspace = new BEWorkspace();
	_Workspaces.append(workspace);
	return workspace;
}
