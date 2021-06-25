#pragma once

#include <QObject>
#include "module_types.h"
#include "global.h"

class Process;

class BEModule : public QObject
{
	Q_OBJECT

public:
	BEModule();
	BEModule(const BEModule& src);
	~BEModule();

	static BEModule* Create(Process* pProc, const MODULEENTRY32& tlh32Entry);

public:
	quint32           ID;         // ģ��ID
	quint64           OwnPID;     // ����ID
	HMODULE           Handle;     // ģ����
	quint64           ModBase;    // Module base
	quint64           ModSize;    // Module size
	quint64           EntryPoint; // Entry point

	QString           FilePath;   // ����·��, �����ļ����ֺ���չ��, ����: c:\windows\ntdll.dll
	QString           FileName;   // �ļ���, ����: ntdll.dll
	QString           FileExt;    // �ļ���չ��, ����: dll
	quint64           FileSize;   // �ļ���С
};


Q_DECLARE_METATYPE(Module);