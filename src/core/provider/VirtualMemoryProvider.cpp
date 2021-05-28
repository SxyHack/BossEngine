#include "VirtualMemoryProvider.h"

VirtualMemoryProvider::VirtualMemoryProvider()
	: BEProvider(nullptr)
	, _ProcessHandle(NULL)
	, _ProcessMinAddress(0x00000000)
	, _ProcessMaxAddress(0x7FFFFFFF)
{
	SYSTEM_INFO sysInfo;
	::GetSystemInfo(&sysInfo);

	_ProcessMinAddress = (ULONG_PTR)sysInfo.lpMinimumApplicationAddress;
	_ProcessMaxAddress = (ULONG_PTR)sysInfo.lpMaximumApplicationAddress;

}

VirtualMemoryProvider::~VirtualMemoryProvider()
{
}
