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
	EScanValueType  ScanValueType;  // 扫描数值的类型, 比如1字节, 2字节等等
	EScanMethod     ScanMethod;     // 扫描方法
	EBaseMode       BaseMode;       // 搜索数值的进制形式
	bool            MemoryWritable;
	bool            MemoryExecutable;
	bool            MemoryCopyOnWrite;
	bool            BaseScan;       // 是否首次扫描或者被叫做基础扫描
	qint64          MemBegAddress;  // 弃用参数
	qint64          MemEndAddress;  // 弃用参数
	qint32          NumberOfProcessors;
	qint64          NumberOfScanTotalBytes;    // 所有的模块需要扫描的总字节数
	QAtomicInt      NumberOfScannedBytes;      // 扫描过的字节数

private:
	QThreadPool    _SearchMemoryPool;
	LIST_CMD       _ListOfCmd;
};


typedef QList<BEWorkspace*> LIST_WORKSPACE;

