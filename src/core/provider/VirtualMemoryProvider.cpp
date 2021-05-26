#include "VirtualMemoryProvider.h"

VirtualMemoryProvider::VirtualMemoryProvider()
	: BEProvider(nullptr)
	, _ProcessHandle(NULL)
	, _ProcessMinAddress(0x00000000)
	, _ProcessMaxAddress(0x7FFFFFFF)
{
	SYSTEM_INFO sysInfo;
	::GetSystemInfo(&sysInfo);

	_ProcessMinAddress = (DWORD)sysInfo.lpMinimumApplicationAddress;
	_ProcessMaxAddress = (DWORD)sysInfo.lpMaximumApplicationAddress;

}

VirtualMemoryProvider::~VirtualMemoryProvider()
{
}
