#include "ModuleRelocation.h"

ModuleRelocation::ModuleRelocation()
	: RVA(0)
	, Type(0)
	, Size(0)
{
}

ModuleRelocation::ModuleRelocation(quint64 rva, quint8 type, qint64 size)
	: RVA(rva)
	, Type(type)
	, Size(size)
{
}

ModuleRelocation::~ModuleRelocation()
{
}

bool ModuleRelocation::Contains(quint64 address) const
{
	return address >= RVA && address < RVA + Size;
}
