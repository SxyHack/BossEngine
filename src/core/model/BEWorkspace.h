#pragma once

#include <QObject>
#include <QList>
#include <QThreadPool>

#include "define.h"

//
// �����ռ�, ���Դ��ڶ�������ռ�
//
class BEWorkspace : public QObject
{
	Q_OBJECT
public:
	BEWorkspace();
	~BEWorkspace();

	void Reset();
	void ScanValue(const QString& valueA, const QString& valueB);

public:
	QString         Name;
	qint32          UniqueID;
	EScanValueType  ScanValueType;
	EScanMethod     ScanMethod;
	EBaseMode       BaseMode;
	bool            MemoryWriteable;
	bool            MemoryExecuteable;
	bool            MemoryCopyOnWrite;
	qint64          ScanBegAddress;
	qint64          ScanEndAddress;
	bool            BaseScan;

private:
	QThreadPool    _ScanThreadPool;
};


typedef QList<BEWorkspace*> LIST_WORKSPACE;

