#pragma once

#include "global.h"

#include <QObject>
#include <QFile>


class MappingFile : public QObject
{
	Q_OBJECT

public:
	enum class Access {
		ACCESS_READ,
		ACCESS_WRITE,
		ACCESS_ALL
	};

public:
	MappingFile(const QString& path, Access access);
	~MappingFile();

private:
	QFile   _File_;
	DWORD   _DesiredAccess_;
	QString _FilePath_;
	Access  _Access_;
};
