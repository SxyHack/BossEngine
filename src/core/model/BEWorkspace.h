#pragma once

#include <QObject>
#include <QList>
#include <QThreadPool>

#include "cmd/BECmdScan.h"

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
	void ScanValueInMemory(const QString& valueA, const QString& valueB);

	//
	// ��ĳ����ַ��ȡ�ڴ�
	//
	bool ReadMemory(quint64 ulAddress, QVariant& value);
	//
	// ��ȡ�ҵ��ĵ�ַ������
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
	EScanValueType  ScanValueType;  // ɨ����ֵ������, ����1�ֽ�, 2�ֽڵȵ�
	ECompareMethod  CompareMethod;  // ɨ�跽��
	EBaseMode       BaseMode;       // ������ֵ�Ľ�����ʽ
	bool            MemoryWritable;
	bool            MemoryExecutable;
	bool            MemoryCopyOnWrite;
	bool            IsBaseScan;       // �Ƿ��״�ɨ����߱���������ɨ��
	qint64          MemBegAddress;  // ���ò���
	qint64          MemEndAddress;  // ���ò���
	qint32          NumberOfProcessors;
	qint64          NumberOfScanTotalBytes;    // ���е�ģ����Ҫɨ������ֽ���
	QAtomicInt      NumberOfScannedBytes;      // ɨ������ֽ���

private:
	QThreadPool    _SearchMemoryPool;
	LIST_CMD       _CmdList;
	BECmd*         _LastCmd;
	BECmd*         _WorkingCmd;
};


typedef QList<BEWorkspace*> LIST_WORKSPACE;

