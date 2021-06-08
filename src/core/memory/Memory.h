#pragma once
#include "core/module/module_types.h"

#include <windows.h>
#include <QString>
#include <QList>

#define PAGE_SHIFT              (12)
//#define PAGE_SIZE               (4096)
#define PAGE_ALIGN(Va)          ((ULONG_PTR)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#define BYTES_TO_PAGES(Size)    (((Size) >> PAGE_SHIFT) + (((Size) & (PAGE_SIZE - 1)) != 0))
#define ROUND_TO_PAGES(Size)    (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

typedef struct {
	MEMORY_BASIC_INFORMATION mbi;         
	QString                  desc;  // ����
} MEMORY_PAGE;

typedef QList<MEMORY_PAGE> LIST_MEMORY_PAGE;

//
// Memory class
//
class BEMemory
{
private:
	BEMemory();

public:
	static BEMemory& Instance();

	//
	// brief ͨ�� windows-sdk ��ȡ������Ч���ڴ�ҳ.
	// param: bListAll �Ƿ��о����е�ҳ��
	// return: bool �ɹ�����true, ʧ�ܷ���false
	// 
	bool LoadPages(bool bListAll = true);

private:

private:
	LIST_MEMORY_PAGE _Pages;
};


#define Memory BEMemory::Instance()