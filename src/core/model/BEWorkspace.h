#pragma once

#include <QObject>
#include <QList>
#include <QThreadPool>

#include "cmd/BECmdSearchValue.h"

//
// 工作空间, 可以存在多个工作空间
//
class BEWorkspace : public QObject
{
	Q_OBJECT
public:
	BEWorkspace();
	~BEWorkspace();

	void Reset();
	void SearchMemory(const QString& valueA, const QString& valueB);


private slots:
	void OnMemorySearchDone();

signals:
	void ES_MemorySearchStarted();
	void ES_MemoryScanDone();
	void ES_MemoryScanning(qint64, qint64);

public:
	QString         Name;
	qint32          UniqueID;
	EScanValueType  ScanValueType;
	EScanMethod     ScanMethod;
	EBaseMode       BaseMode;
	bool            MemoryWriteable;
	bool            MemoryExecuteable;
	bool            MemoryCopyOnWrite;
	qint64          MemBegAddress;
	qint64          ScanEndAddress;
	qint32          NumberOfProcessors;
	bool            BaseScan;
	qint64          NumberOfScanTotalBytes; // 所有的模块需要扫描的总字节数
	QAtomicInt      ScannedBytes;      // 扫描过的字节数

private:
	QThreadPool    _SearchMemoryPool;
	LIST_CMD       _ListOfCmd;
};


typedef QList<BEWorkspace*> LIST_WORKSPACE;

