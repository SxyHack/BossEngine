#pragma once

#include <QObject>
#include <QList>
#include <QThreadPool>

#include "cmd/BECmdSearchValue.h"

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
	EScanValueType  ScanValueType;  // ɨ����ֵ������, ����1�ֽ�, 2�ֽڵȵ�
	EScanMethod     ScanMethod;     // ɨ�跽��
	EBaseMode       BaseMode;       // ������ֵ�Ľ�����ʽ
	bool            MemoryWritable;
	bool            MemoryExecutable;
	bool            MemoryCopyOnWrite;
	bool            BaseScan;       // �Ƿ��״�ɨ����߱���������ɨ��
	qint64          MemBegAddress;  // ���ò���
	qint64          MemEndAddress;  // ���ò���
	qint32          NumberOfProcessors;
	qint64          NumberOfScanTotalBytes;    // ���е�ģ����Ҫɨ������ֽ���
	QAtomicInt      NumberOfScannedBytes;      // ɨ������ֽ���

private:
	QThreadPool    _SearchMemoryPool;
	LIST_CMD       _ListOfCmd;
};


typedef QList<BEWorkspace*> LIST_WORKSPACE;

