#include "BEWorkspace.h"
#include <QMutex>
#include <windows.h>

BEWorkspace::BEWorkspace()
	: QObject(nullptr)
	, UniqueID(-1)
	, CompareMethod(ECompareMethod_Exact)
	, ScanValueType(EScanValueType_4_Byte)
	, BaseMode(EBaseMode_10)
	, IsBaseScan(true)
	, MemoryWritable(true)
	, MemoryExecutable(false)
	, MemoryCopyOnWrite(false)
	, NumberOfScanTotalBytes(0)
	, _LastCmd(nullptr)
	, _WorkingCmd(nullptr)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	MemBegAddress = (qint64)si.lpMinimumApplicationAddress;
	MemEndAddress = (qint64)si.lpMaximumApplicationAddress;
	NumberOfProcessors = si.dwNumberOfProcessors;
}

BEWorkspace::~BEWorkspace()
{
}

void BEWorkspace::Reset()
{
	IsBaseScan = true;
	_LastCmd = nullptr;
	_WorkingCmd = nullptr;

	for (auto cmd : _CmdList) {
		delete cmd;
	}

	_CmdList.clear();
}

void BEWorkspace::ScanValueInMemory(const QString& valueA, const QString& valueB)
{
	auto prev = _CmdList.isEmpty()
		? nullptr
		: qobject_cast<BECmdScan*>(_CmdList.last());

	auto cmd = new BECmdScan(this, valueA, valueB, prev);
	connect(cmd, &BECmd::ES_Started, this, &BEWorkspace::ES_MemorySearchStarted, Qt::QueuedConnection);
	connect(cmd, &BECmd::ES_Done, this, &BEWorkspace::OnMemorySearchDone, Qt::QueuedConnection);
	connect(cmd, &BECmdScan::ES_MemoryScanning, this, &BEWorkspace::ES_MemoryScanning);
	connect(cmd, &BECmdScan::ES_FoundPointer, this, &BEWorkspace::ES_FoundPointer);

	cmd->IsBaseScan = IsBaseScan;
	cmd->start(QThread::HighestPriority);

	_WorkingCmd = cmd;
	_CmdList.append(cmd);

}


bool BEWorkspace::ReadMemory(quint64 ulAddress, QVariant& value)
{
	auto pCmd = qobject_cast<BECmdScan*>(_LastCmd);
	if (pCmd == nullptr)
		return false;

	auto pPtr = pCmd->GetFoundPointer(ulAddress);
	if (pPtr == nullptr)
		return false;

	return pPtr->ReadMemory(value);
}

static QMutex mxGetPointerCount;
quint64 BEWorkspace::GetPointerCount()
{
	QMutexLocker lock(&mxGetPointerCount);

	if (_WorkingCmd == nullptr)
		return 0;

	return _WorkingCmd->GetFoundCount();
}

void BEWorkspace::OnMemorySearchDone(qint64 count)
{
	_LastCmd = _WorkingCmd;
	IsBaseScan = count > 0 ? false : true;
	
	if (count == 0) 
	{
		qDebug("扫描完成, 但是结果为0, 不转换为NEXT");
	}
	else
	{
		qDebug("扫描完成, 转换为NEXT");
	}

	emit ES_MemoryScanDone(count);
}
