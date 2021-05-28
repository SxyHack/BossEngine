#include "BECmdSearchValue.h"
#include "../BEWorkspace.h"
#include "BEngine.h"

#include <QRunnable>
#include <windows.h>
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
	_Workspace->ScannedBytes += value;
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
		ZeroMemory(&ModuleEntry32, sizeof(MODULEENTRY32));
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

		while (_Command->IsRunning() && nMemBegAddr < nMemEndAddr)
		{
			BYTE* lpBuf = new BYTE[ScanValueSize];
			SIZE_T nNumberOfBytesRead = 0;

			if (!::ReadProcessMemory(hProcess, (LPVOID)nMemBegAddr, lpBuf, ScanValueSize, &nNumberOfBytesRead))
			{
				qDebug("[-] 扫描地址: %p", nMemBegAddr);
				nMemBegAddr += 1;
				delete[] lpBuf;
				continue;
			}

			//qDebug("[+] 扫描地址: %p", nMemBegAddr);

			QVariant vMemory;
			if (CheckMemoryIsMatch(lpBuf, vMemory))
			{
				QString qsModName = QString::fromWCharArray(ModuleEntry32.szModule);
				qDebug("[+] 找到数值: %d(%p) %s", vMemory.toInt(), nMemBegAddr, qsModName.toUtf8().data());

				_Command->AddScannedByte(ScanValueSize);
				// 保存地址, 值, 数据类型等等
				nMemBegAddr += ScanValueSize;
			}
			else
			{
				_Command->AddScannedByte(1);

				nMemBegAddr += 1;
			}

			delete[] lpBuf;
		}
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
	MODULEENTRY32   ModuleEntry32;

private:
	BECmdSearchValue* _Command;
};


void BECmdSearchValue::run()
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
			worker->ModuleEntry32 = mod;

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
