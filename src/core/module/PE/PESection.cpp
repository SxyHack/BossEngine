#include "PESection.h"

PESection::PESection()
	: QObject(nullptr)
	, Header(NULL)
{
	FOA = 0;
	RVA = 0;
	ImageAddress = 0;
	ImageSize = 0;
	RawAddress = 0;
	RawSize = 0;
}

PESection::PESection(const PIMAGE_SECTION_HEADER pHeader, quint64 imageBase) : QObject(nullptr)
	, Header(pHeader)
{
	RVA = pHeader->VirtualAddress;
	FOA = pHeader->PointerToRawData;
	ImageAddress = imageBase + RVA;
	ImageSize = pHeader->Misc.VirtualSize;
	RawAddress = pHeader->PointerToRawData;
	RawSize = pHeader->SizeOfRawData;
	Name = QString::fromLocal8Bit((char*)pHeader->Name, IMAGE_SIZEOF_SHORT_NAME);
}

PESection::PESection(const PESection& src)
{
	Header = src.Header;
	RVA = src.RVA;
	FOA = src.FOA;
	ImageAddress = src.ImageAddress;
	ImageSize = src.ImageSize;
	RawAddress = src.RawAddress;
	RawSize = src.RawSize;
	Name = src.Name;
}

PESection::~PESection()
{
}
