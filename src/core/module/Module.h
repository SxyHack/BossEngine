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
	~Module();

private:
	//
	// ��ʼ�� ImageNtHeaders. ȷ��ModBase, ModSize�Ѿ���ʼ��
	//
	NTSTATUS InitImageNtHeader();
	//
	// ö��PE Section
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
	// statics
	static Module* CreateModule(Process* process, const LOAD_DLL_DEBUG_INFO& loadDLL);

public:
	quint64           ModBase; // Module base
	quint64           ModSize; // Module size
	quint64           Entry;   // Entry point
	quint64           Hash;    // Full module name hash
	quint64           ImageBaseHeader;  // ImageBase field in OptionalHeader
	QString           FilePath;  // ����·��, �����ļ����ֺ���չ��, ����: c:\windows\ntdll.dll
	QString           FileName;  // �ļ���, ����: ntdll.dll
	QString           FileExt;   // �ļ���չ��, ����: dll
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


typedef QMap<QRange, Module*> MAP_MOUDLE;
