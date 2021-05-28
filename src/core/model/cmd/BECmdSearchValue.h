#pragma once

#include "BECmd.h"
#include "../define.h"

#include <QThreadPool>
#include <QAtomicInt>

class BEWorkspace;
class ScanWorker;

class BECmdSearchValue : public BECmd
{
	Q_OBJECT

public:
	BECmdSearchValue(BEWorkspace* ws, const QString& valueA, const QString& valueB);
	~BECmdSearchValue();

	void Stop() override;
	bool IsRunning();
	void AddScannedByte(qint32 value);

protected:
	void run() override;

signals:
	void ES_MemoryScanning(qint64, qint64);

public:
	QString         ValueA;
	QString         ValueB;
	bool            IsBaseScan;
	EBaseMode       BaseMode; // 进制模式
	EScanValueType  ScanValueType;
	EScanMethod     ScanMethod;

private:
	QThreadPool    _SearchMemoryPool;
	BEWorkspace*   _Workspace;
	QAtomicInt     _Stopped;            // 中断标志, 用来停止线程
};

