#pragma once
#include "global.h"
#include <QString>
#include <QList>

class ModuleExport
{
public:
	ModuleExport();
	~ModuleExport();

public:  // ����
	DWORD   Ordinal;
	DWORD   RVA;   // ��������ַ(Relative Virtual Address)
	BOOL    Forwarded;
	QString ForwardName;
	QString Name;
	QString UndecoratedName;
};


typedef QList<ModuleExport> LIST_EXPORT;
