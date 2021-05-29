#include "BECmdSearchValue.h"
#include "../BEWorkspace.h"
#include "BEngine.h"

#include <QRunnable>
#include <QVariant>




BECmdSearchValue::BECmdSearchValue(BEWorkspace* ws, const QString& valueA, const QString& valueB) : BECmd()
, IsBaseScan(ws->BaseScan)
, BaseMode(ws->BaseMode)
, ValueA(valueA)
, ValueB(valueB)
, ScanMethod(ws->ScanMethod)
, ScanValueType(ws->ScanValueType)
, _Workspace(ws)
{
}

BECmdSearchValue::~BECmdSearchValue()
{
}


void BECmdSearchValue::Stop()
{
	_Stopped.storeRelaxed(0);
}

bool BECmdSearchValue::IsRunning()
{
	return _Stopped.loadRelaxed() == 1;
}

void BECmdSearchValue::AddScannedByte(qint32 value)
{
	_Workspace->NumberOfScannedBytes += value;
}

//
// 扫描工作线程
//
class ScanWorker : public QRunnable
{
public:
	ScanWorker(BECmdSearchValue* command)
		: _Command(command)
		, BaseMode(EBaseMode_10)
		, IsBaseScan(true)
		, MemBegAddress(0)
		, ScanMethod(EScanMethod_Exact)
		, ScanValueType(EScanValueType_4_Byte)
		, ScanValueSize(4)
		, MemRegionSize(0)
	{
		ZeroMemory(&MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION));
	}

	~ScanWorker()
	{
		qDebug("~");
	}

protected:
	QVariant MakeFromMemory(BYTE* lpBuf, SIZE_T nSize)
	{
		switch (ScanValueType)
		{
		case EScanValueType_Byte:
			return QVariant((BYTE)*lpBuf);
		case EScanValueType_2_Byte:
			return QVariant(*(WORD*)lpBuf);
		case EScanValueType_4_Byte:
			return QVariant(*(DWORD32*)lpBuf);
		case EScanValueType_8_Byte:
			return QVariant(*(DWORD64*)lpBuf);
		case EScanValueType_Float:
			return QVariant(*(FLOAT*)lpBuf);
		case EScanValueType_Double:
			return QVariant(*(DOUBLE*)lpBuf);
		case EScanValueType_String:
			return QVariant(QString::fromLocal8Bit((char*)lpBuf, nSize));
		case EScanValueType_All:
			qWarning("not support EScanValueType_All");
			break;
		}

		return QVariant();
	}

	QVariant MakeFromInput(const QString& value)
	{
		switch (ScanValueType)
		{
		case EScanValueType_Byte:
			return QVariant((BYTE)value.toUInt());
		case EScanValueType_2_Byte:
			return QVariant((WORD)value.toUInt());
		case EScanValueType_4_Byte:
			return QVariant((DWORD32)value.toUInt());
		case EScanValueType_8_Byte:
			return QVariant((DWORD64)value.toULongLong());
		case EScanValueType_Float:
			return QVariant((FLOAT)value.toFloat());
		case EScanValueType_Double:
			return QVariant((DOUBLE)value.toDouble());
		case EScanValueType_String:
			return QVariant(value);
		case EScanValueType_All:
			qWarning("not support EScanValueType_All");
			break;
		}

		return QVariant();
	}

	bool CheckMemoryIsMatch(BYTE* lpBuf, QVariant& vMemory)
	{
		QVariant vInputA;
		QVariant vinputB;

		switch (ScanMethod)
		{
		case EScanMethod_Exact:
			vInputA = MakeFromInput(ValueA);
			vMemory = MakeFromMemory(lpBuf, ScanValueSize);
			return vInputA == vMemory;
		case EScanMethod_Greater:
			break;
		case EScanMethod_Less:
			break;
		case EScanMethod_Between:
			break;
		case EScanMethod_Unknown:
			break;
			//------------------------------ base scan --------------------------------------------
		case EScanMethod_MoreValue:
			break;
		case EScanMethod_AddValue:
			break;
		case EScanMethod_Smaller:
			break;
		case EScanMethod_CutValue:
			break;
		case EScanMethod_Changed:
			break;
		case EScanMethod_Equal:
			break;
		case EScanMethod_FirstDiff:
			break;
		default:
			break;
		}

		return false;
	}

	void HandleBaseSearch()
	{
		WinExtras extras;
		HANDLE hProcess = Engine.GetProcessHandle();
		qint64 nRemainSize = MemRegionSize;
		qint64 nMemBegAddr = MemBegAddress;
		qint64 nMemEndAddr = MemBegAddress + MemRegionSize;

		qint64 nNumberOfFound = 0;
		SIZE_T nNumberOfBytesRead = 0;
		BYTE*  lpBuffer = new BYTE[ScanValueSize];

		while (_Command->IsRunning() && nMemBegAddr < nMemEndAddr)
		{
			ZeroMemory(lpBuffer, ScanValueSize);

			if (!::ReadProcessMemory(hProcess, (LPVOID)nMemBegAddr, lpBuffer, ScanValueSize, &nNumberOfBytesRead))
			{
				auto qsProtect = extras.FormatMemoryProtection(MemoryInformation.Protect);
				auto qsMemType = extras.FormatMemoryType(MemoryInformation.Type);
				auto qsMemState = extras.FormatMemoryState(MemoryInformation.State);
				qWarning("无法读取内存:(%p), %s %s %s", nMemBegAddr, 
					qsProtect.toUtf8().data(), 
					qsMemType.toUtf8().data(), 
					qsMemState.toUtf8().data());
				nMemBegAddr += ScanValueSize;
				continue;
			}

			//qDebug("[+] 扫描地址: %p", nMemBegAddr);

			QVariant vMemory;
			if (CheckMemoryIsMatch(lpBuffer, vMemory))
			{
				qDebug("[+] 找到数值: %d(%p) %s", vMemory.toInt(), nMemBegAddr, ModuleName.toUtf8().data());

				// 保存地址, 值, 数据类型等等
			}
			else
			{
			}

			_Command->AddScannedByte(ScanValueSize);
			nMemBegAddr += ScanValueSize;
		}

		delete[] lpBuffer;
	}

	void run() override
	{
		if (IsBaseScan)
		{
			qDebug("扫描区块:[%p, %p]", MemBegAddress, MemBegAddress + MemRegionSize);
			HandleBaseSearch();
		}
		else
		{

		}
	}

public:
	QString         ValueA;
	QString         ValueB;
	bool            IsBaseScan;
	EBaseMode       BaseMode; // 进制模式
	EScanValueType  ScanValueType;
	EScanMethod     ScanMethod;
	qint32          ScanValueSize;
	qint64          MemBegAddress;
	qint64          MemRegionSize;
	QString         ModuleName;
	MEMORY_BASIC_INFORMATION MemoryInformation;

private:
	BECmdSearchValue* _Command;
};


void BECmdSearchValue::run()
{
	const auto nRegionSize = (DWORD64)1024 * 20;
	const auto numberOfProcessors = qMin(_Workspace->NumberOfProcessors, 10);
	const auto byteSize = GetScanValueTypeSize(ScanValueType);

	auto hProcess = Engine.GetProcessHandle();
	_Stopped.storeRelaxed(1); // 设置运行中状态
	_SearchMemoryPool.setMaxThreadCount(numberOfProcessors);
	_Workspace->NumberOfScanTotalBytes = 0;

	DWORD64 ulQueryAddr = 0, dwBaseAddr = 0;
	MEMORY_BASIC_INFORMATION mbi;
	SIZE_T nMBISize = sizeof(MEMORY_BASIC_INFORMATION);
	ZeroMemory(&mbi, nMBISize);


	while (::VirtualQueryEx(hProcess, (LPCVOID)ulQueryAddr, &mbi, nMBISize))
	{
		dwBaseAddr = ulQueryAddr;
		ulQueryAddr += mbi.RegionSize;

		TCHAR szModName[MAX_PATH];
		auto size = sizeof(szModName) / sizeof(TCHAR);
		::GetModuleBaseName(hProcess, (HMODULE)mbi.BaseAddress, szModName, size);

		QString qsModName = QString::fromWCharArray(szModName);
		if (!CheckIsIncludeModule(qsModName))
		{
			continue;
		}

		if (!CheckIsValidRegion(mbi))
		{
			continue;
		}

		DWORD64 dwBegAddr = dwBaseAddr;
		DWORD64 dwModSize = mbi.RegionSize;
		DWORD64 dwEndAddr = dwBaseAddr + dwModSize;

		while (dwBegAddr < dwEndAddr)
		{
			auto dwRealSize = qMin(nRegionSize, mbi.RegionSize);
			auto worker = new ScanWorker(this);
			worker->IsBaseScan = this->IsBaseScan;
			worker->ValueA = ValueA;
			worker->ValueB = ValueB;
			worker->BaseMode = this->BaseMode;
			worker->ScanMethod = this->ScanMethod;
			worker->ScanValueType = this->ScanValueType;
			worker->ScanValueSize = byteSize;
			worker->MemBegAddress = dwBegAddr;
			worker->MemRegionSize = dwRealSize;
			worker->ModuleName = qsModName;
			worker->MemoryInformation = mbi;

			_SearchMemoryPool.start(worker);
			_Workspace->NumberOfScanTotalBytes += dwRealSize;

			qDebug("模块(%s) %p+%x=%p 加入扫描...", qsModName.toUtf8().data(),
				dwBegAddr, dwRealSize, dwBegAddr + dwRealSize);

			dwBegAddr += dwRealSize;
			dwModSize -= dwRealSize;
		}
	}

	emit ES_Started();
	_SearchMemoryPool.waitForDone();
	emit ES_Done();
}

bool BECmdSearchValue::CheckIsIncludeModule(const QString& modName)
{
	for (auto& mod : Engine.GetIncludeModules())
	{
		auto qsModName = QString::fromWCharArray(mod.szModule);
		if (modName.compare(qsModName, Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}

	return false;
}

bool BECmdSearchValue::CheckIsValidRegion(const MEMORY_BASIC_INFORMATION& mbi)
{
	bool bValid = false;
	bool bGuard = mbi.Protect & PAGE_GUARD;
	bool bNoAccess = mbi.Protect & PAGE_NOACCESS;
	bool bNoCache = mbi.Protect & PAGE_NOCACHE;
	bool bWriteCombine = mbi.Protect & PAGE_WRITECOMBINE;

	bValid = (mbi.State == MEM_COMMIT);
	bValid = bValid && !bGuard;
	bValid = bValid && !bNoAccess;
	bValid = bValid && !bNoCache;
	bValid = bValid && !bWriteCombine;
	bValid = bValid && mbi.Protect & PAGE_READONLY;

	if (_Workspace->MemoryWritable)
	{
		bValid = bValid || mbi.Protect & PAGE_READWRITE;
		bValid = bValid || mbi.Protect & PAGE_WRITECOPY;
		bValid = bValid || mbi.Protect & PAGE_EXECUTE_READWRITE;
		bValid = bValid || mbi.Protect & PAGE_EXECUTE_WRITECOPY;
	}

	if (_Workspace->MemoryExecutable)
	{
		bValid = bValid || mbi.Protect & PAGE_EXECUTE;
		bValid = bValid || mbi.Protect & PAGE_EXECUTE_READ;
		bValid = bValid || mbi.Protect & PAGE_EXECUTE_READWRITE;
		bValid = bValid || mbi.Protect & PAGE_EXECUTE_WRITECOPY;
	}

	if (_Workspace->MemoryCopyOnWrite)
	{
		bValid = bValid || mbi.Protect & PAGE_WRITECOPY;
		bValid = bValid || mbi.Protect & PAGE_EXECUTE_WRITECOPY;
	}

	return bValid;
}

void BECmdSearchValue::EnumVirtualMemoryByModule()
{
	const auto nRegionSize = (DWORD)1024 * 20;
	const auto numberOfProcessors = qMin(_Workspace->NumberOfProcessors, 10);
	const auto byteSize = GetScanValueTypeSize(ScanValueType);

	auto modules = Engine.GetIncludeModules();
	_Stopped.storeRelaxed(1); // 设置运行中状态
	_SearchMemoryPool.setMaxThreadCount(numberOfProcessors);
	_Workspace->NumberOfScanTotalBytes = 0;

	for (auto& mod : modules)
	{
		ULONG_PTR ulModBegAddr = (ULONG_PTR)mod.modBaseAddr;
		ULONG_PTR ulModEndAddr = ulModBegAddr + mod.modBaseSize;
		DWORD     dwModSize = mod.modBaseSize;
		while (ulModBegAddr < ulModEndAddr)
		{
			auto dwRealSize = qMin(nRegionSize, dwModSize);
			auto worker = new ScanWorker(this);
			worker->IsBaseScan = this->IsBaseScan;
			worker->ValueA = ValueA;
			worker->ValueB = ValueB;
			worker->BaseMode = this->BaseMode;
			worker->ScanMethod = this->ScanMethod;
			worker->ScanValueType = this->ScanValueType;
			worker->ScanValueSize = byteSize;
			worker->MemBegAddress = ulModBegAddr;
			worker->MemRegionSize = dwRealSize;
			//worker->ModuleEntry32 = mod;

			_SearchMemoryPool.start(worker);
			_Workspace->NumberOfScanTotalBytes += dwRealSize;

			ulModBegAddr += dwRealSize;
			dwModSize -= dwRealSize;
		}

		auto qsModName = QString::fromWCharArray(mod.szModule);
		qDebug("模块(%s) %p+%x=%p 加入扫描队列...", qsModName.toUtf8().data(),
			mod.modBaseAddr,
			mod.modBaseSize,
			mod.modBaseAddr + mod.modBaseSize);
	}

	emit ES_Started();

	_SearchMemoryPool.waitForDone();

	emit ES_Done();

}
