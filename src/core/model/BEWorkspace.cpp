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
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	
	ScanBegAddress = (qint64)si.lpMinimumApplicationAddress;
	ScanEndAddress = (qint64)si.lpMaximumApplicationAddress;
}

BEWorkspace::~BEWorkspace()
{
}

void BEWorkspace::Reset()
{
	BaseScan = true;
}


void BEWorkspace::ScanValue(const QString& valueA, const QString& valueB)
{
	if (BaseScan)
	{

	}
	else
	{

	}
}
