#pragma once

#include <QObject>
#include <QList>

//
// ModuleSection class
//
class ModuleSection : public QObject
{
	Q_OBJECT

public:
	ModuleSection();
	ModuleSection(quint64 addr, quint64 size, const QString& name);
	ModuleSection(const ModuleSection& src);
	~ModuleSection();

public:
	quint64 Address;
	quint64 Size;
	QString Name;
};

// typedef
typedef QList<ModuleSection> LIST_MODSECTION;