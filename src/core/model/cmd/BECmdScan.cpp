#include "BECmdScan.h"
#include "BEngine.h"
#include "settings/ScanSettings.h"
#include "../BEWorkspace.h"

#include <QRunnable>
#include <QVariant>
#include <QMutex>


//
// 扫描工作线程
//
class ScanWorker : public QRunnable
{
public:
	ScanWorker(BECmdScan* command)
		: _Command(command)
		, BaseMode(EBaseMode_10)
		, IsBaseScan(true)
		, MemBegAddress(0)
		, CompareMethod(ECompareMethod_Exact)
		, ScanValueType(EScanValueType_4_Byte)
		, ScanValueSize(4)
		, MemRegionSize(0)
		, ModuleBaseAddr(0)
		, ModuleRegionSize(0)
	{
		ZeroMemory(&MBI, sizeof(MEMORY_BASIC_INFORMATION));
	}

	~ScanWorker()
	{
	}

protected:

	void RunBaseScan()
	{
		WinExtras extras;
		HANDLE hProcess = Engine.GetProcessHandle();
		DWORD64 nRemainSize = MemRegionSize;
		DWORD64 nBegPtrAddr = MemBegAddress;
		DWORD64 nEndPtrAddr = MemBegAddress + MemRegionSize;

		SIZE_T nNumberOfBytesRead = 0;
		BYTE* pBuffer = new BYTE[ScanValueSize];
		auto   compare = BECompare::Create(CompareMethod, ScanValueType);
		compare->SetInputs(ValueA, ValueB);

		while (_Command->IsRunning() && nBegPtrAddr < nEndPtrAddr)
		{
			ZeroMemory(pBuffer, ScanValueSize);

			if (!::ReadProcessMemory(hProcess, (LPVOID)nBegPtrAddr, pBuffer, ScanValueSize, &nNumberOfBytesRead))
			{
				auto qsProtect = extras.FormatMemoryProtection(MBI.Protect);
				auto qsMemType = extras.FormatMemoryType(MBI.Type);
				auto qsMemState = extras.FormatMemoryState(MBI.State);
				qWarning("无法读取内存:%s(%p), %s, %s, %s", ModuleName.toUtf8().data(),
					nBegPtrAddr,
					qsProtect.toUtf8().data(),
					qsMemState.toUtf8().data(),
					qsMemType.toUtf8().data());
				break; // 因为该内存页的属性是一样的, 只要是无法读取可以全部跳过.
			}

			if (compare->BaseMatch(pBuffer, ScanValueSize))
			{
				auto qsProtect = extras.FormatMemoryProtection(MBI.Protect);
				auto qsMemType = extras.FormatMemoryType(MBI.Type);
				auto qsMemState = extras.FormatMemoryState(MBI.State);
				qDebug("[+] 找到数值(%p):%d {%s, %s, %s}", nBegPtrAddr, *(WORD*)pBuffer,
					qsProtect.toUtf8().data(),
					qsMemState.toUtf8().data(),
					qsMemType.toUtf8().data());

				BEPointer* ptr = new BEPointer(ScanValueType);
				ptr->Value = compare->GetValue();
				ptr->ModBaseAddr = ModuleBaseAddr;
				ptr->ModName = ModuleName;
				ptr->Address = nBegPtrAddr;
				ptr->hProcess = hProcess;
				ptr->IsBaseScan = IsBaseScan;
				ptr->CompareMethod = CompareMethod;
				ptr->ValueSize = ScanValueSize;
				ptr->MBI = MBI;

				_Command->AppendPointer(ptr);
			}

			_Command->AddScannedByte(ScanValueSize);
			nBegPtrAddr += ScanValueSize;
		}

		delete   compare;
		delete[] pBuffer;
	}

	void run() override
	{
		RunBaseScan();
	}

public:
	QString         ValueA;  // 输入值A
	QString         ValueB;  // 输入值B, 只有比较方式是介于...两值之间的才有用
	bool            IsBaseScan;
	EBaseMode       BaseMode; // 进制模式
	EScanValueType  ScanValueType;
	ECompareMethod  CompareMethod;
	INT32           ScanValueSize;
	DWORD64         MemBegAddress;
	DWORD64         MemRegionSize;
	QString         ModuleName;
	DWORD64         ModuleBaseAddr;
	DWORD64         ModuleRegionSize;
	MEMORY_BASIC_INFORMATION MBI;

private:
	BECmdScan* _Command;
};


//
// class BECmdScan
//
BECmdScan::BECmdScan(BEWorkspace* ws, const QString& valueA, const QString& valueB, BECmdScan* prev) : BECmd()
, IsBaseScan(ws->IsBaseScan)
, BaseMode(ws->BaseMode)
, ValueA(valueA)
, ValueB(valueB)
, CompareMethod(ws->CompareMethod)
, ScanValueType(ws->ScanValueType)
, _Workspace(ws)
, _Prev(prev)
{
}

BECmdScan::~BECmdScan()
{
	qDebug("~");
}


void BECmdScan::Stop()
{
	_Stopped.storeRelaxed(0);
}

qint64 BECmdScan::GetFoundCount()
{
	return _FoundPtrList.length();
}

bool BECmdScan::IsRunning()
{
	return _Stopped.loadRelaxed() == 1;
}

void BECmdScan::AddScannedByte(qint32 value)
{
	_Workspace->NumberOfScannedBytes += value;
}

void BECmdScan::AppendPointer(BEPointer* pPtr)
{
	static QMutex mutex;
	QMutexLocker lock(&mutex); // 线程互斥锁

	pPtr->Index = _FoundPtrList.count();
	_FoundPtrList.append(pPtr);
	_FoundPtrMap.insert(pPtr->Address, pPtr);
	emit ES_FoundPointer(pPtr, _FoundPtrList.count());
}

void BECmdScan::RemovePointer(BEPointer* pPtr)
{
	if (pPtr == nullptr)
		return;

	_FoundPtrList.removeOne(pPtr);
	_FoundPtrMap.remove(pPtr->Address);
}

LIST_POINTER& BECmdScan::GetPointers()
{
	return _FoundPtrList;
}

BEPointer* BECmdScan::GetFoundPointer(quint64 address)
{
	return _FoundPtrMap.value(address);
}

void BECmdScan::run()
{
	if (IsBaseScan)
		RunBase();
	else
		RunNext();
}

void BECmdScan::RunBase()
{
	const auto numberOfProcessors = qMin(_Workspace->NumberOfProcessors, 10);
	const auto byteSize = GetScanValueTypeSize(ScanValueType);
	const auto nRegionSize = (DWORD64)100000 * byteSize;

	auto hProcess = Engine.GetProcessHandle();
	_Stopped.storeRelaxed(1); // 设置运行中状态
	_SearchMemoryPool.setMaxThreadCount(numberOfProcessors);
	_Workspace->NumberOfScanTotalBytes = 0;

	WinExtras extras;
	MEMORY_BASIC_INFORMATION mbi;
	DWORD64 ulQueryAddr = 0, ulBaseAddr = 0;
	SIZE_T  nMBISize = sizeof(MEMORY_BASIC_INFORMATION);
	ZeroMemory(&mbi, nMBISize);

	emit ES_Started();

	while (::VirtualQueryEx(hProcess, (LPCVOID)ulQueryAddr, &mbi, nMBISize))
	{
		ulBaseAddr = ulQueryAddr;
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
			auto qsProtect = extras.FormatMemoryProtection(mbi.Protect);
			auto qsMemType = extras.FormatMemoryType(mbi.Type);
			auto qsMemState = extras.FormatMemoryState(mbi.State);
			qWarning("非法页内存:(%s)%x [%s %s %s]", qsModName.toUtf8().data(), ulBaseAddr,
				qsProtect.toUtf8().data(),
				qsMemState.toUtf8().data(),
				qsMemType.toUtf8().data());
			continue;
		}

		//qDebug("可用页内存:(%s)%x [%s %s %s]", qsModName.toUtf8().data(), dwBaseAddr,
		//	qsProtect.toUtf8().data(),
		//	qsMemState.toUtf8().data(),
		//	qsMemType.toUtf8().data());

		DWORD64 dwBegAddr = ulBaseAddr;
		DWORD64 dwModSize = mbi.RegionSize;
		DWORD64 dwEndAddr = ulBaseAddr + dwModSize;

		while (dwBegAddr < dwEndAddr)
		{
			auto dwRealSize = qMin(nRegionSize, mbi.RegionSize);
			auto worker = new ScanWorker(this);
			worker->IsBaseScan = this->IsBaseScan;
			worker->ValueA = ValueA;
			worker->ValueB = ValueB;
			worker->BaseMode = this->BaseMode;
			worker->CompareMethod = this->CompareMethod;
			worker->ScanValueType = this->ScanValueType;
			worker->ScanValueSize = byteSize;
			worker->MemBegAddress = dwBegAddr;
			worker->MemRegionSize = dwRealSize;
			worker->ModuleName = qsModName;
			worker->MBI = mbi;

			_SearchMemoryPool.start(worker);

			// 这里的计算是为了显示进度条, 进度的更新由UI层驱动.
			// UI会创建计时器, 每20ms查询当前进度, 这样的设计性能优
			_Workspace->NumberOfScanTotalBytes += dwRealSize;

			dwBegAddr += dwRealSize;
			dwModSize -= dwRealSize;
		}
	}

	_SearchMemoryPool.waitForDone();
	emit ES_Done(_FoundPtrList.count());
}

void BECmdScan::RunNext()
{
	if (_Prev == nullptr) {
		emit ES_ScanError();
		return;
	}
	auto listPointer = _Prev->GetPointers();
	auto compare = BECompare::Create(CompareMethod, ScanValueType);
	compare->SetInputs(ValueA, ValueB);

	emit ES_Started();

	for (auto ptr : listPointer)
	{
		if (!ptr->ReadMemory())
		{
			continue;
		}

		if (compare->NextMatch(ptr->Value))
		{
			AppendPointer(ptr);
		}
	}

	emit ES_Done(_FoundPtrList.count());
}

bool BECmdScan::CheckIsIncludeModule(const QString& modName)
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

bool BECmdScan::CheckIsValidRegion(const MEMORY_BASIC_INFORMATION& mbi)
{
	bool bValid = false;
	bool bWritable = (mbi.Protect & PAGE_READWRITE) || (mbi.Protect & PAGE_EXECUTE_READWRITE);
	bool bExecutable = (mbi.Protect & PAGE_EXECUTE) ||
		(mbi.Protect & PAGE_EXECUTE_READ) ||
		(mbi.Protect & PAGE_EXECUTE_READWRITE) ||
		(mbi.Protect & PAGE_EXECUTE_WRITECOPY);
	bool bCopyOnWrite = (mbi.Protect & PAGE_WRITECOPY) || (mbi.Protect & PAGE_EXECUTE_WRITECOPY);

	bool bGuard = mbi.Protect & PAGE_GUARD;
	bool bNoAccess = mbi.Protect & PAGE_NOACCESS;
	bool bNoCache = mbi.Protect & PAGE_NOCACHE;
	bool bWriteCombine = mbi.Protect & PAGE_WRITECOMBINE;

	if (bGuard && !SCAN_SETTING.GetPageGuardEnable()) {
		return false;
	}

	if (bWriteCombine && !SCAN_SETTING.GetPageWriteCombineEnable())
		return false;

	if (bNoAccess || bNoCache) {
		return false;
	}

	if (mbi.Protect == 0)
		return false;

	if (mbi.State == MEM_FREE)
		return false;

	if (mbi.Type == MEM_MAPPED && !SCAN_SETTING.GetMemMappedEnable())  // 排除
		return false;

	if (!_Workspace->MemoryWritable && bWritable)
		return false;

	if (!_Workspace->MemoryExecutable && bExecutable)
		return false;

	if (!_Workspace->MemoryCopyOnWrite && bCopyOnWrite)
		return false;

	return true;
}

//void BECmdScan::EnumVirtualMemoryByModule()
//{
//	const auto nRegionSize = (DWORD)1024 * 20;
//	const auto numberOfProcessors = qMin(_Workspace->NumberOfProcessors, 10);
//	const auto byteSize = GetScanValueTypeSize(ScanValueType);
//
//	auto modules = Engine.GetIncludeModules();
//	_Stopped.storeRelaxed(1); // 设置运行中状态
//	_SearchMemoryPool.setMaxThreadCount(numberOfProcessors);
//	_Workspace->NumberOfScanTotalBytes = 0;
//
//	for (auto& mod : modules)
//	{
//		ULONG_PTR ulModBegAddr = (ULONG_PTR)mod.modBaseAddr;
//		ULONG_PTR ulModEndAddr = ulModBegAddr + mod.modBaseSize;
//		DWORD     dwModSize = mod.modBaseSize;
//		while (ulModBegAddr < ulModEndAddr)
//		{
//			auto dwRealSize = qMin(nRegionSize, dwModSize);
//			auto worker = new ScanWorker(this);
//			worker->IsBaseScan = this->IsBaseScan;
//			worker->ValueA = ValueA;
//			worker->ValueB = ValueB;
//			worker->BaseMode = this->BaseMode;
//			worker->CompareMethod = this->CompareMethod;
//			worker->ScanValueType = this->ScanValueType;
//			worker->ScanValueSize = byteSize;
//			worker->MemBegAddress = ulModBegAddr;
//			worker->MemRegionSize = dwRealSize;
//
//			_SearchMemoryPool.start(worker);
//			_Workspace->NumberOfScanTotalBytes += dwRealSize;
//
//			ulModBegAddr += dwRealSize;
//			dwModSize -= dwRealSize;
//		}
//
//		auto qsModName = QString::fromWCharArray(mod.szModule);
//		qDebug("模块(%s) %p+%x=%p 加入扫描队列...", qsModName.toUtf8().data(),
//			mod.modBaseAddr,
//			mod.modBaseSize,
//			mod.modBaseAddr + mod.modBaseSize);
//	}
//
//	emit ES_Started();
//
//	_SearchMemoryPool.waitForDone();
//
//	emit ES_Done();
//
//}
