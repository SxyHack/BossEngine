#include "BEWorkspace.h"
#include <windows.h>

BEWorkspace::BEWorkspace()
	: QObject(nullptr)
	, UniqueID(-1)
	, ScanMethod(EScanMethod_Exact)
	, ScanValueType(EScanValueType_4_Byte)
	, BaseMode(EBaseMode_10)
	, BaseScan(true)
	, MemoryCopyOnWrite(false)
	, MemoryWriteable(true)
	, MemoryExecuteable(true)
	, NumberOfScanTotalBytes(0)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	
	MemBegAddress = (qint64)si.lpMinimumApplicationAddress;
	ScanEndAddress = (qint64)si.lpMaximumApplicationAddress;
	NumberOfProcessors = si.dwNumberOfProcessors;
}

BEWorkspace::~BEWorkspace()
{
}

void BEWorkspace::Reset()
{
	BaseScan = true;
}

void BEWorkspace::SearchMemory(const QString& valueA, const QString& valueB)
{
	auto cmd = new BECmdSearchValue(this, valueA, valueB);
	connect(cmd, &BECmd::ES_Started, this, &BEWorkspace::ES_MemorySearchStarted, Qt::QueuedConnection);
	connect(cmd, &BECmd::ES_Done, this, &BEWorkspace::OnMemorySearchDone);
	connect(cmd, &BECmdSearchValue::ES_MemoryScanning, this, &BEWorkspace::ES_MemoryScanning);

	cmd->start(QThread::HighestPriority);
}

void BEWorkspace::OnMemorySearchDone()
{
	BECmdSearchValue* cmd = qobject_cast<BECmdSearchValue*>(QObject::sender());
	if (cmd == nullptr)
		return;

	_ListOfCmd.append(cmd);
	emit ES_MemoryScanDone();
}
