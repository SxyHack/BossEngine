#pragma once

#include <QObject>
#include <QList>
#include <QThreadPool>

#include "cmd/BECmdScan.h"

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
	void ScanValueInMemory(const QString& valueA, const QString& valueB);

	//
	// 让某个地址读取内存
	//
	bool ReadMemory(quint64 ulAddress, QVariant& value);
	//
	// 获取找到的地址总数量
	//
	quint64 GetPointerCount();

private slots:
	void OnMemorySearchDone(qint64);

signals:
	void ES_MemorySearchStarted();
	void ES_MemoryScanDone(qint64);
	void ES_MemoryScanning(qint64, qint64);
	void ES_FoundPointer(BEPointer*, qint64);

public:
	QString         Name;
	qint32          UniqueID;       
	EScanValueType  ScanValueType;  // 扫描数值的类型, 比如1字节, 2字节等等
	ECompareMethod  CompareMethod;  // 扫描方法
	EBaseMode       BaseMode;       // 搜索数值的进制形式
	bool            MemoryWritable;
	bool            MemoryExecutable;
	bool            MemoryCopyOnWrite;
	bool            IsBaseScan;       // 是否首次扫描或者被叫做基础扫描
	qint64          MemBegAddress;  // 弃用参数
	qint64          MemEndAddress;  // 弃用参数
	qint32          NumberOfProcessors;
	qint64          NumberOfScanTotalBytes;    // 所有的模块需要扫描的总字节数
	QAtomicInt      NumberOfScannedBytes;      // 扫描过的字节数

private:
	QThreadPool    _SearchMemoryPool;
	LIST_CMD       _CmdList;
	BECmd*         _LastCmd;
	BECmd*         _WorkingCmd;
};


typedef QList<BEWorkspace*> LIST_WORKSPACE;

