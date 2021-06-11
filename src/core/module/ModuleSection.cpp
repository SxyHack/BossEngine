#include "ModuleSection.h"

ModuleSection::ModuleSection(quint64 addr, quint64 size, const QString& name)
	: QObject(nullptr)
	, Address(addr)
	, Size(size)
{
}

ModuleSection::ModuleSection()
	: QObject(nullptr)
	, Address(0)
	, Size(0)
{
}

ModuleSection::ModuleSection(const ModuleSection& src)
	: QObject(nullptr)
{
	Address = src.Address;
	Size = src.Size;
	Name = src.Name;
}

ModuleSection::~ModuleSection()
{
}
