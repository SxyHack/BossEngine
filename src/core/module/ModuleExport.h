#pragma once
#include "global.h"
#include <QString>
#include <QList>

class ModuleExport
{
public:
	ModuleExport();
	~ModuleExport();

public:  // 变量
	DWORD   Ordinal;
	DWORD   RVA;   // 相对虚拟地址(Relative Virtual Address)
	BOOL    Forwarded;
	QString ForwardName;
	QString Name;
	QString UndecoratedName;
};


typedef QList<ModuleExport> LIST_EXPORT;
