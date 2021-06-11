#pragma once

#include <QList>

//
// ModuleRelocation class
//
class ModuleRelocation
{
public:
	ModuleRelocation();
	ModuleRelocation(quint64 rva, quint8 type, qint64 size);
	~ModuleRelocation();

	quint64 RVA;
	quint8  Type;
	qint64  Size;

	bool Contains(quint64 address) const;
};


typedef QList<ModuleRelocation> LIST_RELOCATION;
