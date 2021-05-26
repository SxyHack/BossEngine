#pragma once

#include "BEProvider.h"

class VirtualMemoryProvider : public BEProvider
{
	Q_OBJECT

public:
	VirtualMemoryProvider();
	~VirtualMemoryProvider();



private:
	HANDLE _ProcessHandle;
	DWORD  _ProcessMinAddress;
	DWORD  _ProcessMaxAddress;
};
