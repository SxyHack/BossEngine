#pragma once
#include <QString>
#include <QList>

//
//
//
class ModuleImport
{
public:
	ModuleImport();
	~ModuleImport();

	quint64 IATRelativeVirtualAddress;
	quint32 ModuleIndex;
	quint64 Ordinal;
	QString Name;
	QString UndecoratedName;
};


typedef QList<ModuleImport> LIST_IMPORT;