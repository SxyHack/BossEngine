#include "Memory.h"
#include "BEngine.h"

static BEMemory instance;

BEMemory& BEMemory::Instance()
{
	return instance;
}

bool BEMemory::LoadPages(bool bListAll)
{
	quint64 numBytes = 0;
	quint64 ulQueryAddr = 0;
	quint64 ulAllocationBase = 0;
	HANDLE  hProcess = Engine.GetProcessHandle();
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
					? QString().sprintf("±£Áô (%p)", ulAllocationBase)
					: "±£Áô";
			}
			else if (false) // TODO: !ModNameFromAddr
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