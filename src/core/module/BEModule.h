#pragma once

#include <QObject>
#include "module_types.h"
#include "PESection.h"

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
	quint32           ID;         // 模块ID
	quint64           OwnPID;     // 进程ID
	HMODULE           Handle;     // 模块句柄
	quint64           ModBase;    // Module base
	quint64           ModSize;    // Module size
	quint64           EntryPoint; // Entry point

	QString           FilePath;   // 完整路径, 包括文件名字和扩展名, 比如: c:\windows\ntdll.dll
	QString           FileName;   // 文件名, 比如: ntdll.dll
	QString           FileExt;    // 文件扩展名, 比如: dll
	quint64           FileSize;   // 文件大小
	quint64           FileMapBase;

	MODULE_PARTY      Party;

	// PE headers
	PIMAGE_DOS_HEADER      ImageDosHeader;   // Dos Header
	PIMAGE_NT_HEADERS      ImageNtHeaders;   // NT Headers
	PIMAGE_FILE_HEADER     ImageFileHeader;
	PIMAGE_OPTIONAL_HEADER ImageOptHeader;
	PIMAGE_SECTION_HEADER  ImageFirstSection;
	quint64                ImageBase;
	PE_MACHINE             Machine;

	// Sections
	qint32                 NumberOfSections;
	LIST_PESECTION         Sections;
};


Q_DECLARE_METATYPE(BEModule);