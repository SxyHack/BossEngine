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
	ULONG_PTR  _ProcessMinAddress;
	ULONG_PTR  _ProcessMaxAddress;
};
