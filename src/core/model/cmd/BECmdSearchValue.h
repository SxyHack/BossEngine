#pragma once

#include "BECmd.h"
#include "../define.h"

#include <QThreadPool>
#include <QAtomicInt>

#include <windows.h>
#include <psapi.h>

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

	bool CheckIsIncludeModule(const QString& modName);
	bool CheckIsValidRegion(const MEMORY_BASIC_INFORMATION& mbi);

	void EnumVirtualMemoryByModule();

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

