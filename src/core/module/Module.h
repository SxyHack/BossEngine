#pragma once

#include "global.h"
#include "module_types.h"
#include "symbol/symbol_types.h"
#include "ModuleSection.h"
#include "ModuleExport.h"
#include "ModuleImport.h"
#include "ModuleRelocation.h"
#include <QObject>
#include <QMap>
#include <QFile>

// Macros to safely access IMAGE_NT_HEADERS fields since the compile-time typedef of this struct may not match the actual file bitness.
// Never access OptionalHeader.xx values directly unless they have the same size and offset on 32 and 64 bit. IMAGE_FILE_HEADER fields are safe to use
#define IMAGE32(NtHeaders) ((NtHeaders) != nullptr && (NtHeaders)->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
#define IMAGE64(NtHeaders) ((NtHeaders) != nullptr && (NtHeaders)->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
#define HEADER_FIELD(NtHeaders, Field) (IMAGE64(NtHeaders) \
    ? ((PIMAGE_NT_HEADERS64)(NtHeaders))->OptionalHeader.Field : (IMAGE32(NtHeaders) \
        ? ((PIMAGE_NT_HEADERS32)(NtHeaders))->OptionalHeader.Field \
        : 0))
#define THUNK_VAL(NtHeaders, Ptr, Val) (IMAGE64(NtHeaders) \
    ? ((PIMAGE_THUNK_DATA64)(Ptr))->Val : (IMAGE32(NtHeaders) \
        ? ((PIMAGE_THUNK_DATA32)(Ptr))->Val \
        : 0))

class Process;

class Module : public QObject
{
	Q_OBJECT

public:
	Module();
	Module(const Module& src);
	~Module();

private:
	//
	// 初始化 ImageNtHeaders. 
	// 确保 ModBase, ModSize, MappingVA 已经成功解析
	//
	NTSTATUS InitImageNtHeader();
	//
	// 枚举PE Section
	//
	void ListPESection();
	void ListExports();
	void ListImports();
	void ListTLSCallbacks();
	void ListRelocations();
	void ReadDebugDir();
	void ReadExceptionDir();

	quint64 RVAToOffset(quint64 base, quint64 rva);
	bool IsDebugSupport(PIMAGE_DEBUG_DIRECTORY pEntry);
	static QString FormatDebugType(DWORD type);

public:
	// static
	static Module* CreateModule(Process* process, LPVOID lpBaseOfDLL);
	static Module* Create(Process* pProc, const MODULEENTRY32& tlh32Entry);

public:
	quint64           ModBase;    // Module base
	quint64           ModSize;    // Module size
	quint64           Entry;      // Entry point
	quint64           ImageBase;  // ImageBase field in OptionalHeader
	quint64           FileHash;   // Full module name hash
	QString           FilePath;   // 完整路径, 包括文件名字和扩展名, 比如: c:\windows\ntdll.dll
	QString           FileName;   // 文件名, 比如: ntdll.dll
	QString           FileExt;    // 文件扩展名, 比如: dll
	quint64           FileSize;   // 文件大小
	quint64           MappingVA;
	PIMAGE_NT_HEADERS ImageNtHeaders;
	MODULE_PARTY      Party;

	DWORD             ExportOrdinalBase = 0; //ordinal - 'exportOrdinalBase' = index in 'exports'

	QString           PdbSignature;
	QString           PdbFile;
	PDB_VALIDATION    PdbValidation;
	QStringList       PdbPaths;

protected:
	LIST_MODSECTION  _Sections;
	LIST_EXPORT      _Exports;
	LIST_INDEX_NAME  _ExportSortByIndexName;
	QList<quint64>   _ExportSortByRVA;

	QStringList      _ImportModules;
	LIST_IMPORT      _Imports;
	QList<quint64>   _ImportSortByRVA; //index in 'imports', sorted by rva

	QList<quint64>   _ListTLS;

	LIST_RELOCATION  _Relocations;
};


typedef QMap<QRange, Module*> RANGE_MAP_MOUDLE;
typedef QMap<QString, Module*> NAME_MAP_MODULE;


Q_DECLARE_METATYPE(Module);