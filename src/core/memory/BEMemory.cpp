#include "BEMemory.h"
#include "BEngine.h"


BEMemory& BEMemory::Instance()
{
	static BEMemory instance;
	return instance;
}

bool BEMemory::LoadPages(bool bListAll)
{
	quint64 numBytes = 0;
	quint64 ulQueryAddr = 0;
	quint64 ulAllocationBase = 0;
	Process* pProcess = Engine.GetProcess();
	if (pProcess == nullptr) 
	{
		qWarning("打开进程失败");
		return false;
	}

	HANDLE  hProcess = pProcess->Handle();
	MEMORY_BASIC_INFORMATION mbi;
	ZeroMemory(&mbi, sizeof(mbi));

	while (::VirtualQueryEx(hProcess, (LPCVOID)ulQueryAddr, &mbi, sizeof(mbi)))
	{
		bool bFree = (mbi.State == MEM_FREE);
		bool bReserved = (mbi.State == MEM_RESERVE);
		bool bMapped = (mbi.Type == MEM_MAPPED);
		bool bPrevReserved = _Pages.isEmpty() ? false : _Pages.last().mbi.State == MEM_RESERVE;

		if (bFree) continue;

		if (bReserved || bPrevReserved || ulAllocationBase != quint64(mbi.AllocationBase))
		{
			ulAllocationBase = (quint64)mbi.AllocationBase;

			MEMORY_PAGE page;
			ZeroMemory(&page, sizeof(MEMORY_PAGE));
			CopyMemory(&page.mbi, &mbi, sizeof(mbi));

			if (bReserved)
			{
				page.desc = (quint64(page.mbi.BaseAddress) != ulAllocationBase)
					? QString().sprintf("保留 (%p)", ulAllocationBase)
					: "保留";
			}
			else if (pProcess->GetModuleNameByAddr(ulQueryAddr, page.desc)) // TODO: !ModNameFromAddr
			{
				TCHAR szMappedName[MAX_MODULE_SIZE] = L"";
				if (bMapped && (GetMappedFileName(hProcess, mbi.AllocationBase, szMappedName, MAX_MODULE_SIZE) != 0))
				{
					page.desc = QString::fromWCharArray(szMappedName);
				}
			}

			_Pages.append(page);
		}
		else
		{
			if (_Pages.length() > 0)
				_Pages.last().mbi.RegionSize += mbi.RegionSize;
		}

		quint64 nextPageAddr = (quint64)mbi.BaseAddress + mbi.RegionSize;
		if (nextPageAddr <= ulQueryAddr)
			break;

		ulQueryAddr = nextPageAddr;
	}

	return false;
}

bool BEMemory::IsCanonicalAddress(quint64 address)
{
#ifndef _WIN64
	// 32-bit mode only supports 4GB max, so limits are
	// not an issue
	return true;
#else
	// The most-significant 16 bits must be all 1 or all 0.
	// (64 - 16) = 48bit linear address range.
	//
	// 0xFFFF800000000000 = Significant 16 bits set
	// 0x0000800000000000 = 48th bit set
	return (((address & 0xFFFF800000000000) + 0x800000000000) & ~0x800000000000) == 0;
#endif //_WIN64

}
