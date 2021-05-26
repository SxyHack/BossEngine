#include "BEngine.h"
#include "undocument.h"

Q_DECLARE_METATYPE(PROCESSENTRY32)


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

void BEngine::EnumProcess(const QString& filter)
{
	BOOL ret = FALSE;
	DWORD dwRow = 0;
	PROCESSENTRY32 entry32 = { 0 };
	entry32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		qWarning("CreateToolhelp32Snapshot FAIL, %0x08X", GetLastError());
		return;
	}

	emit sgEnumProcessPrepare();

	ret = ::Process32First(hSnap, &entry32);
	do 
	{
		if (entry32.th32ProcessID <= 0)
			continue;

		QString processName = QString::fromWCharArray(entry32.szExeFile);

		if (filter.isEmpty() || processName.contains(filter)) {
			emit sgEnumProcess(dwRow++, entry32);
		}
	} while (ret = ::Process32Next(hSnap, &entry32));

	emit sgEnumProcessDone(dwRow);
	CloseHandle(hSnap);
}

BOOL BEngine::OpenProcess(DWORD pid)
{
	//NTSTATUS status;
	//OBJECT_ATTRIBUTES objAttr;
	//CLIENT_ID clientID;
	//InitializeObjectAttributes(&objAttr, NULL, 0, NULL, NULL);
	//clientID.UniqueProcess = (HANDLE)pid;
	//clientID.UniqueThread = 0;

	_AttachProcessID = pid;
	_AttachProcessHandle = ::OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
	if (_AttachProcessHandle == NULL)
	{
		_LastErrorCode = GetLastError();
		_LastErrorMessage = _WinExtras.FormatLastError(_LastErrorCode);
		qCritical("OpenProcess[%d] failed, ERROR:0x%x", pid, _LastErrorCode);
		return FALSE;
	}

	return EnumModules();
}

BOOL BEngine::EnumModules()
{
	_AttachProcessModules.clear();
	// ²éÑ¯Ä£¿é
	if (!_WinExtras.QueryProcessModulesTH(_AttachProcessID, _AttachProcessModules))
	{
		qCritical("QueryProcessModulesTH Failed");
		return FALSE;
	}

	return TRUE;
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
