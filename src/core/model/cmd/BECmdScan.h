#pragma once

#include "BECmd.h"
#include "../BEValue.h"
#include "../compare/BECompare.h"

#include <QThreadPool>
#include <QAtomicInt>

#include <psapi.h>

class BEWorkspace;
class ScanWorker;

class BECmdScan : public BECmd
{
	Q_OBJECT

public:
	BECmdScan(BEWorkspace* ws, const QString& valueA, const QString& valueB, BECmdScan* prev = nullptr);
	~BECmdScan();

	void Stop() override;
	qint64 GetFoundCount() override;

	bool IsRunning();
	void AddScannedByte(qint32 numOfByte);
	void AppendPointer(BEPointer* pPtr);
	void RemovePointer(BEPointer* pPtr);

	LIST_POINTER& GetPointers();

	//
	// 获取找到的地址指针
	//
	BEPointer* GetFoundPointer(quint64 address);

protected:
	void run() override;
	void RunBase();
	void RunNext();

	bool CheckIsIncludeModule(const QString& modName);
	bool CheckIsValidRegion(const MEMORY_BASIC_INFORMATION& mbi);

	//void EnumVirtualMemoryByModule();

signals:
	void ES_MemoryScanning(qint64, qint64);
	void ES_FoundPointer(BEPointer*, qint64);
	void ES_ScanError();

public:
	QString         ValueA;
	QString         ValueB;
	bool            IsBaseScan;
	EBaseMode       BaseMode; // 进制模式
	EScanValueType  ScanValueType;
	ECompareMethod  CompareMethod;

private:
	QThreadPool  _SearchMemoryPool;
	BEWorkspace* _Workspace;
	BECompare*   _Compare;
	QAtomicInt   _Stopped;            // 中断标志, 用来停止线程
	LIST_POINTER _FoundPtrList;
	MAP_POINTER  _FoundPtrMap;
	BECmdScan*   _Prev;
};

