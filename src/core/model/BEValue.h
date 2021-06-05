#pragma once

#include <QObject>
#include <QList>
#include <QMap>
#include <QVariant>

#include <windows.h>

#include "define.h"

class BEPointer : public QObject
{
	Q_OBJECT

public:
	BEPointer(EScanValueType type);
	~BEPointer();

	bool operator==(const QString& input);
	bool operator==(const QVariant& input);

	bool ReadMemory(QVariant& value);
	bool ReadMemory();

public:
	static QVariant MakeFrByte(EScanValueType valueType, BYTE* pBuffer, SIZE_T nSize);
	static QVariant MakeFrString(EScanValueType valueType, const QString& input);

public:
	QVariant       Value;
	EScanValueType ValueType;
	qint32         ValueSize;
	ECompareMethod CompareMethod;
	quint64        Address;
	HANDLE         hProcess;
	quint64        ModBaseAddr;
	QString        ModName;
	qint32         Index;
	bool           IsBaseScan;

	MEMORY_BASIC_INFORMATION MBI;
};

typedef QList<BEPointer*>       LIST_POINTER;
typedef QMap<quint64, BEPointer*> MAP_POINTER;