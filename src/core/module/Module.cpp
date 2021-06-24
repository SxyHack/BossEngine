#include "Module.h"
#include "Process.h"
#include "WinExtras.h"
#include "libs/ntdll/ntdll.h"
#include "core/memory/Memory.h"
#include "symbol/symbol_undecorator.h"
#include "settings/GlobalSettings.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <Shlwapi.h>

Module::Module() : QObject(nullptr)
, ModBase(0)
, ModSize(0)
, Hash(0)
, Entry(0)
, ImageBaseHeader(0)
, MappingVA(0)
, ImageNtHeaders(NULL)
, Party(MODULE_PARTY::USER)
{
}

Module::~Module()
{
}

NTSTATUS Module::InitImageNtHeader()
{
	__try
	{
		if (MappingVA == 0)
			return STATUS_INVALID_PARAMETER;

		if (ModSize < sizeof(IMAGE_DOS_HEADER))
			return STATUS_INVALID_IMAGE_FORMAT;

		const PIMAGE_DOS_HEADER pDosHeaders = (PIMAGE_DOS_HEADER)MappingVA;
		if (pDosHeaders->e_magic != IMAGE_DOS_SIGNATURE)
			return STATUS_INVALID_IMAGE_FORMAT;

		const ULONG e_lfanew = pDosHeaders->e_lfanew;
		const ULONG sizeOfPeSignature = sizeof('PE00');
		if (e_lfanew >= ModSize ||
			e_lfanew >= (ULONG_MAX - sizeOfPeSignature - sizeof(IMAGE_FILE_HEADER)) ||
			(e_lfanew + sizeOfPeSignature + sizeof(IMAGE_FILE_HEADER)) >= ModSize)
			return STATUS_INVALID_IMAGE_FORMAT;

		PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(MappingVA + e_lfanew);
		// RtlImageNtHeaderEx verifies that the range does not cross the UM <-> KM boundary here,
		// but it would cost a syscall to query this address as it varies between OS versions // TODO: or do we already have this info somewhere?
		if (!BEMemory::IsCanonicalAddress((quint64)pNtHeaders + sizeof(IMAGE_NT_HEADERS)))
			return STATUS_INVALID_IMAGE_FORMAT;
		if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
			return STATUS_INVALID_IMAGE_FORMAT;

		// Get the entry point
		this->ImageNtHeaders = pNtHeaders;

		quint64 modOEP = HEADER_FIELD(ImageNtHeaders, AddressOfEntryPoint);
		BOOL bIsDLL = ((ImageNtHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL);

		// Fix a problem where the OEP is set to zero (non-existent).
		// OEP can't start at the PE header/offset 0 -- except if module is an EXE.
		this->Entry = modOEP + ModBase;
		this->ImageBaseHeader = (quint64)HEADER_FIELD(ImageNtHeaders, ImageBase);
		if (modOEP == 0 && bIsDLL)
		{
			Entry = 0;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return GetExceptionCode();
	}

	return STATUS_SUCCESS;
}

void Module::ListPESection()
{
	if (ImageNtHeaders == nullptr)
	{
		qCritical("Must be call InitImageNtHeader before.");
		return;
	}
	PIMAGE_SECTION_HEADER pNtSection = IMAGE_FIRST_SECTION(ImageNtHeaders);

	auto numberOfSections = ImageNtHeaders->FileHeader.NumberOfSections;

	for (int i = 0; i < numberOfSections; i++)
	{
		ModuleSection section;
		section.Address = pNtSection->VirtualAddress + ModBase;
		section.Size = pNtSection->Misc.VirtualSize;
		section.Name = QString::fromLocal8Bit((char*)pNtSection->Name, IMAGE_SIZEOF_SHORT_NAME);
		qDebug("%s: %p(%d)", section.Name.toUtf8().data(), section.Address, section.Size);
		_Sections.push_back(section);
		pNtSection++;
	}
}

void Module::ListExports()
{
	ULONG exportDirSize = 0;

	try
	{
		auto exportDir = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData((PVOID)MappingVA,
			FALSE,
			IMAGE_DIRECTORY_ENTRY_EXPORT,
			&exportDirSize);

		if (exportDirSize == 0 || exportDir == nullptr ||
			(ULONG_PTR)exportDir + exportDirSize > MappingVA + ModSize || // Check if exportDir fits into the mapped area
			(ULONG_PTR)exportDir + exportDirSize < (ULONG_PTR)exportDir ||// Check for ULONG_PTR wraparound (e.g. when exportDirSize == 0xfffff000)
			exportDir->NumberOfFunctions == 0)
			return;
		DWORD64 totalFunctionSize = exportDir->NumberOfFunctions * sizeof(ULONG_PTR);
		if (totalFunctionSize / exportDir->NumberOfFunctions != sizeof(ULONG_PTR) || // Check for overflow
			totalFunctionSize > ModSize) // Check for impossible number of exports
			return;

		auto addressOfFunctionsOffset = RVAToOffset(0, exportDir->AddressOfFunctions);
		if (!addressOfFunctionsOffset)
			return;
		auto addressOfFunctions = PDWORD(addressOfFunctionsOffset + MappingVA);

		auto addressOfNamesOffset = RVAToOffset(0, exportDir->AddressOfNames);
		auto addressOfNames = PDWORD(addressOfNamesOffset ? addressOfNamesOffset + MappingVA : 0);

		auto addressOfNameOrdinalsOffset = RVAToOffset(0, exportDir->AddressOfNameOrdinals);
		auto addressOfNameOrdinals = PWORD(addressOfNameOrdinalsOffset ? addressOfNameOrdinalsOffset + MappingVA : 0);

		// Do not reserve memory based on untrusted input
		//Info.exports.reserve(exportDir->NumberOfFunctions);
		ExportOrdinalBase = exportDir->Base;

		// TODO: 'invalid address' below means an RVA that is obviously invalid, like being greater than SizeOfImage.
		// In that case rva2offset will return a VA of 0 and we can ignore it. However the ntdll loader (and this code)
		// will still crash on corrupt or malicious inputs that are seemingly valid. Find out how common this is
		// (i.e. does it warrant wrapping everything in try/except?) and whether there are better solutions.
		// Note that we're loading this file because the debuggee did; that makes it at least somewhat plausible that we will also survive
		for (DWORD i = 0; i < exportDir->NumberOfFunctions; i++)
		{
			// Check if addressOfFunctions[i] is valid
			ULONG_PTR target = (ULONG_PTR)addressOfFunctions + i * sizeof(DWORD);
			if (target > MappingVA + ModSize || target < (ULONG_PTR)addressOfFunctions)
			{
				continue;
			}

			// It is possible the AddressOfFunctions contain zero RVAs. GetProcAddress for these ordinals returns zero.
			// "The reason for it is to assign a particular ordinal to a function." - NTCore
			if (!addressOfFunctions[i])
				continue;

			ModuleExport entry;
			//_Exports.emplace_back();
			//auto& entry = _Exports.back();
			entry.Ordinal = i + exportDir->Base;
			entry.RVA = addressOfFunctions[i];
			const auto entryVA = RVAToOffset(MappingVA, entry.RVA);
			entry.Forwarded = entryVA >= (ULONG64)exportDir && entryVA < (ULONG64)exportDir + exportDirSize;
			if (entry.Forwarded)
			{
				auto forwardNameOffset = RVAToOffset(0, entry.RVA);
				if (forwardNameOffset) // Silent ignore (1) by ntdll loader: invalid forward names or addresses of forward names
					entry.ForwardName = QString::fromLocal8Bit((const char*)(forwardNameOffset + MappingVA));
			}
			_Exports.push_back(entry);
		}

		for (DWORD i = 0; i < exportDir->NumberOfNames; i++)
		{
			// Check if addressOfNameOrdinals[i] is valid
			ULONG_PTR target = (ULONG_PTR)addressOfNameOrdinals + i * sizeof(WORD);
			if (target > MappingVA + ModSize || target < (ULONG_PTR)addressOfNameOrdinals)
			{
				continue;
			}

			DWORD index = addressOfNameOrdinals[i];
			if (index < exportDir->NumberOfFunctions) // Silent ignore (2) by ntdll loader: bogus AddressOfNameOrdinals indices
			{
				// Check if addressOfNames[i] is valid
				target = (ULONG_PTR)addressOfNames + i * sizeof(DWORD);
				if (target > MappingVA + ModSize || target < (ULONG_PTR)addressOfNames)
				{
					continue;
				}

				auto nameOffset = RVAToOffset(0, addressOfNames[i]);
				if (nameOffset) // Silent ignore (3) by ntdll loader: invalid names or addresses of names
				{
					// Info.exports has excluded some invalid exports, so addressOfNameOrdinals[i] is not equal to
					// the index of Info.exports. We need to iterate over Info.exports.
					for (size_t j = 0; j < _Exports.size(); j++)
					{
						if (index + exportDir->Base == _Exports[j].Ordinal)
						{
							_Exports[j].Name = QString::fromLocal8Bit((const char*)(nameOffset + MappingVA));
							break;
						}
					}
				}
			}
		}

		// give some kind of name to ordinal functions
		for (size_t i = 0; i < _Exports.size(); i++)
		{
			if (_Exports[i].Name.isEmpty())
				_Exports[i].Name = QString("Ordinal#%1").arg(_Exports[i].Ordinal);
		}

		// prepare sorted vectors
		_ExportSortByIndexName.clear();
		_ExportSortByRVA.clear();
		for (size_t i = 0; i < _Exports.size(); i++)
		{
			IndexName indexName;
			indexName.index = i;
			indexName.name = _Exports[i].Name;
			_ExportSortByIndexName.push_back(indexName);
			_ExportSortByRVA.push_back(i);
			//_ExportSortByIndexName[i].index = i;
			//_ExportSortByIndexName[i].name = _Exports[i].Name;
			//_ExportSortByRVA[i] = i;
		}

		std::sort(_ExportSortByIndexName.begin(), _ExportSortByIndexName.end());
		std::sort(_ExportSortByRVA.begin(), _ExportSortByRVA.end(), [=](size_t a, size_t b)
			{
				return _Exports.at(a).RVA < _Exports.at(b).RVA;
			});

		// undecorate names
		for (auto& x : _Exports)
		{
			if (!x.Name.isEmpty())
			{
				UndecorateName(x.Name, x.UndecoratedName);
			}
		}
	}
	catch (...)
	{
		qCritical("Exception while getting module info, please report...");
		_Exports.clear();
		_ExportSortByIndexName.clear();
		_ExportSortByRVA.clear();
		ExportOrdinalBase = 0;
	}
}

void Module::ListImports()
{
	try
	{
		ULONG importDirSize = 0;
		auto importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData((PVOID)MappingVA,
			FALSE,
			IMAGE_DIRECTORY_ENTRY_IMPORT,
			&importDirSize);
		if (importDirSize == 0 || importDescriptor == nullptr ||
			(ULONG_PTR)importDescriptor + importDirSize > MappingVA + ModSize || // Check if importDescriptor fits into the mapped area
			(ULONG_PTR)importDescriptor + importDirSize < (ULONG_PTR)importDescriptor) // Check for ULONG_PTR wraparound (e.g. when importDirSize == 0xfffff000)
			return;

		const ULONG64 ordinalFlag = IMAGE64(ImageNtHeaders) ? IMAGE_ORDINAL_FLAG64 : IMAGE_ORDINAL_FLAG32;

		for (size_t moduleIndex = 0; importDescriptor->Name != 0; ++importDescriptor, ++moduleIndex)
		{
			auto moduleNameOffset = RVAToOffset(0, importDescriptor->Name);
			if (!moduleNameOffset) // If the module name VA is invalid, the loader crashes with an access violation. Try to avoid this
				break;

			// Prefer OFTs over FTs. If they differ, the FT is a bounded import and has a 0% chance of being correct due to ASLR
			auto thunkOffset = RVAToOffset(0, importDescriptor->OriginalFirstThunk != 0
				? importDescriptor->OriginalFirstThunk
				: importDescriptor->FirstThunk);

			// If there is no FT, the loader ignores the descriptor and moves on to the next DLL instead of crashing. Wise move
			if (importDescriptor->FirstThunk == 0)
				continue;

			_ImportModules.push_back((const char*)(moduleNameOffset + MappingVA));
			unsigned char* thunkData = (unsigned char*)MappingVA + thunkOffset;

			for (auto IAT_RVA = importDescriptor->FirstThunk;
				THUNK_VAL(ImageNtHeaders, thunkData, u1.AddressOfData) != 0;
				thunkData += IMAGE64(ImageNtHeaders) ? sizeof(IMAGE_THUNK_DATA64) : sizeof(IMAGE_THUNK_DATA32), IAT_RVA += IMAGE64(ImageNtHeaders) ? sizeof(ULONG64) : sizeof(DWORD))
			{
				// Get AddressOfData, check whether the ordinal flag was set, and then strip it because the RVA is not valid with it set
				ULONG64 addressOfDataValue = THUNK_VAL(ImageNtHeaders, thunkData, u1.AddressOfData);
				const bool ordinalFlagSet = (addressOfDataValue & ordinalFlag) == ordinalFlag; // NB: both variables are ULONG64 to force this test to be 64 bit
				addressOfDataValue &= ~ordinalFlag;

				auto addressOfDataOffset = RVAToOffset(0, addressOfDataValue);
				if (!addressOfDataOffset && !ordinalFlagSet) // Invalid entries are ignored. Of course the app will crash if it ever calls the function, but whose fault is that?
					continue;

				ModuleImport entry;
				entry.IATRelativeVirtualAddress = IAT_RVA;
				entry.ModuleIndex = moduleIndex;

				auto importByName = PIMAGE_IMPORT_BY_NAME(addressOfDataOffset + MappingVA);
				if (!ordinalFlagSet && importByName->Name[0] != '\0')
				{
					// Import by name
					entry.Name = QString::fromLocal8Bit((const char*)importByName->Name);
					entry.Ordinal = -1;
				}
				else
				{
					// Import by ordinal
					entry.Ordinal = THUNK_VAL(ImageNtHeaders, thunkData, u1.Ordinal) & 0xffff;
					entry.Name = QString("Ordinal#%1").arg(entry.Ordinal);
				}

				_Imports.push_back(entry);
			}
		}

		// prepare sorted vectors
		_ImportSortByRVA.clear();
		for (size_t i = 0; i < _Imports.size(); i++)
			_ImportSortByRVA.push_back(i);

		std::sort(_ImportSortByRVA.begin(), _ImportSortByRVA.end(), [=](size_t a, size_t b)
			{
				return _Imports[a].IATRelativeVirtualAddress < _Imports[b].IATRelativeVirtualAddress;
			});

		// undecorate names
		for (auto& i : _Imports)
		{
			UndecorateName(i.Name, i.UndecoratedName);
		}
	}
	catch (...)
	{
		qCritical("Exception while getting module info, please report...");
		_Imports.clear();
		_ImportSortByRVA.clear();
		_ImportModules.clear();
	}
}

void Module::ListTLSCallbacks()
{
	try
	{
		_ListTLS.clear();
		// Get the TLS directory
		ULONG tlsDirSize;
		auto tlsDir = (PIMAGE_TLS_DIRECTORY)RtlImageDirectoryEntryToData((PVOID)MappingVA,
			FALSE,
			IMAGE_DIRECTORY_ENTRY_TLS,
			&tlsDirSize);
		if (tlsDir == nullptr /*|| tlsDirSize == 0*/ || // The loader completely ignores the directory size. Setting it to 0 is an anti-debug trick
			(ULONG_PTR)tlsDir + tlsDirSize > MappingVA + ModSize || // Check if tlsDir fits into the mapped area
			(ULONG_PTR)tlsDir + tlsDirSize < (ULONG_PTR)tlsDir) // Check for ULONG_PTR wraparound (e.g. when tlsDirSize == 0xfffff000)
			return;

		ULONG64 addressOfCallbacks = IMAGE64(ImageNtHeaders)
			? ((PIMAGE_TLS_DIRECTORY64)tlsDir)->AddressOfCallBacks
			: (ULONG64)((PIMAGE_TLS_DIRECTORY32)tlsDir)->AddressOfCallBacks;
		if (!addressOfCallbacks)
			return;

		auto imageBase = HEADER_FIELD(ImageNtHeaders, ImageBase);
		auto tlsArrayOffset = RVAToOffset(0, tlsDir->AddressOfCallBacks - imageBase);
		if (!tlsArrayOffset)
			return;

		// TODO: proper bounds checking
		auto tlsArray = PULONG_PTR(tlsArrayOffset + MappingVA);
		while (*tlsArray)
		{
			_ListTLS.push_back(quint64(*tlsArray++ - imageBase + ModBase));
		}
	}
	catch (...)
	{
		qCritical("Exception while getting module info, please report...");
	}
}

void Module::ListRelocations()
{
	try
	{
		_Relocations.clear();
		// Ignore files with relocation info stripped
		if ((ImageNtHeaders->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) == IMAGE_FILE_RELOCS_STRIPPED)
			return;
		// Get address and size of base relocation table
		ULONG totalBytes;
		auto baseRelocBlock = (PIMAGE_BASE_RELOCATION)RtlImageDirectoryEntryToData((PVOID)MappingVA,
			FALSE,
			IMAGE_DIRECTORY_ENTRY_BASERELOC,
			&totalBytes);
		if (baseRelocBlock == nullptr || totalBytes == 0 ||
			(ULONG_PTR)baseRelocBlock + totalBytes > MappingVA + ModSize || // Check if baseRelocBlock fits into the mapped area
			(ULONG_PTR)baseRelocBlock + totalBytes < (ULONG_PTR)baseRelocBlock) // Check for ULONG_PTR wraparound (e.g. when totalBytes == 0xfffff000)
			return;

		// Until we reach the end of the relocation table
		while (totalBytes > 0)
		{
			ULONG blockSize = baseRelocBlock->SizeOfBlock;
			if (blockSize == 0 || blockSize > totalBytes) // The loader allows incorrect relocation dir sizes/block counts, but it won't relocate the image
			{
				qCritical("Invalid relocation block for module %s%s!",
					FileName.toUtf8().data(),
					FileExt.toUtf8().data());
				return;
			}

			// Process the relocation block
			totalBytes -= blockSize;
			blockSize = (blockSize - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
			PUSHORT nextOffset = (PUSHORT)((PCHAR)baseRelocBlock + sizeof(IMAGE_BASE_RELOCATION));

			while (blockSize--)
			{
				const auto type = (UCHAR)((*nextOffset) >> 12);
				const auto offset = (USHORT)(*nextOffset & 0xfff);

				if (baseRelocBlock->VirtualAddress + offset > MappingVA + HEADER_FIELD(ImageNtHeaders, SizeOfImage))
				{
					qCritical("Invalid relocation entry for module %s%s!",
						FileName.toUtf8().data(),
						FileExt.toUtf8().data());
					return;
				}

				switch (type)
				{
				case IMAGE_REL_BASED_HIGHLOW:
					_Relocations.push_back(ModuleRelocation(baseRelocBlock->VirtualAddress + offset, type, sizeof(ULONG)));
					break;
				case IMAGE_REL_BASED_DIR64:
					_Relocations.push_back(ModuleRelocation(baseRelocBlock->VirtualAddress + offset, type, sizeof(ULONG64)));
					break;
				case IMAGE_REL_BASED_HIGH:
				case IMAGE_REL_BASED_LOW:
				case IMAGE_REL_BASED_HIGHADJ:
					_Relocations.push_back(ModuleRelocation(baseRelocBlock->VirtualAddress + offset, type, sizeof(USHORT)));
					break;
				case IMAGE_REL_BASED_ABSOLUTE:
				case IMAGE_REL_BASED_RESERVED: // IMAGE_REL_BASED_SECTION; ignored by loader
				case IMAGE_REL_BASED_MACHINE_SPECIFIC_7: // IMAGE_REL_BASED_REL32; ignored by loader
					break;
				default:
					qCritical("Illegal relocation type 0x%02X for module %s%s!",
						FileName.toUtf8().data(),
						FileExt.toUtf8().data());
					return;
				}
				++nextOffset;
			}
			baseRelocBlock = (PIMAGE_BASE_RELOCATION)nextOffset;
		}

		std::sort(_Relocations.begin(), _Relocations.end(), [](ModuleRelocation const& a, ModuleRelocation const& b)
			{
				return a.RVA < b.RVA;
			});

	}
	catch (...)
	{
		qCritical("Exception while getting module info, please report...");
	}
}


void Module::ReadDebugDir()
{
	// Get the debug directory and its size
	ULONG debugDirSize;
	auto debugDir = (PIMAGE_DEBUG_DIRECTORY)RtlImageDirectoryEntryToData((PVOID)MappingVA,
		FALSE,
		IMAGE_DIRECTORY_ENTRY_DEBUG,
		&debugDirSize);
	if (debugDirSize == 0 || debugDir == nullptr ||
		(ULONG_PTR)debugDir + debugDirSize > MappingVA + ModSize || /* Check if debugDir fits into the mapped area */
		(ULONG_PTR)debugDir + debugDirSize < (ULONG_PTR)debugDir) /* Check for ULONG_PTR wraparound (e.g. when debugDirSize == 0xfffff000) */
	{
		return;
	}

	// Iterate over entries until we find a CV one or the end of the directory
	PIMAGE_DEBUG_DIRECTORY entry = debugDir;
	while (debugDirSize >= sizeof(IMAGE_DEBUG_DIRECTORY))
	{
		if (IsDebugSupport(entry))
			break;

		const auto typeName = FormatDebugType(entry->Type);

		/*dprintf("IMAGE_DEBUG_DIRECTORY:\nCharacteristics: %08X\nTimeDateStamp: %08X\nMajorVersion: %04X\nMinorVersion: %04X\nType: %s\nSizeOfData: %08X\nAddressOfRawData: %08X\nPointerToRawData: %08X\n",
				debugDir->Characteristics, debugDir->TimeDateStamp, debugDir->MajorVersion, debugDir->MinorVersion, typeName, debugDir->SizeOfData, debugDir->AddressOfRawData, debugDir->PointerToRawData);*/
		qCritical("Skipping unsupported debug type %s in module %s%s...\n", typeName,
			FileName.toUtf8().data(),
			FileExt.toUtf8().data());
		entry++;
		debugDirSize -= sizeof(IMAGE_DEBUG_DIRECTORY);
	}

	if (!IsDebugSupport(entry))
	{
		qCritical("Did not find any supported debug types in module %s%s!\n",
			FileName.toUtf8().data(),
			FileExt.toUtf8().data());
		return;
	}

	// At this point we know the entry is a valid CV one
	ULONG_PTR offset = 0;
	if (entry->AddressOfRawData)
	{
		offset = (ULONG_PTR)RVAToOffset(0, entry->AddressOfRawData);
	}
	else if (entry->PointerToRawData)
	{
		offset = entry->PointerToRawData;
	}
	auto cvData = (unsigned char*)(MappingVA + offset);
	auto signature = *(DWORD*)cvData;
	if (signature == '01BN')
	{
		auto cv = (CV_INFO_PDB20*)cvData;
		PdbSignature = QString().sprintf("%X%X", cv->Signature, cv->Age);
		PdbFile = QString::fromLocal8Bit((char*)cv->PdbFileName, entry->SizeOfData - FIELD_OFFSET(CV_INFO_PDB20, PdbFileName));
		PdbValidation.Signature = cv->Signature;
		PdbValidation.Age = cv->Age;
	}
	else if (signature == 'SDSR')
	{
		auto cv = (CV_INFO_PDB70*)cvData;
		PdbSignature = QString().sprintf("%08X%04X%04X%s%X",
			cv->Signature.Data1,
			cv->Signature.Data2,
			cv->Signature.Data3,
			QString::number(*(quint64*)cv->Signature.Data4, 16),
			cv->Age);
		PdbFile = QString::fromLocal8Bit((const char*)cv->PdbFileName, entry->SizeOfData - FIELD_OFFSET(CV_INFO_PDB70, PdbFileName));
		memcpy(&PdbValidation.Guid, &cv->Signature, sizeof(GUID));
		PdbValidation.Signature = 0;
		PdbValidation.Age = cv->Age;
	}

	//dprintf("%s%s pdbSignature: %s, pdbFile: \"%s\"\n", Info.name, Info.extension, Info.pdbSignature.c_str(), Info.pdbFile.c_str());

	if (!PdbFile.isEmpty())
	{
		// Get the directory/filename from the debug directory PDB path
		QFileInfo qfPdbFile(PdbFile);
		QString fileDir = qfPdbFile.absolutePath();
		QString fileName = qfPdbFile.baseName();

		// TODO: this order is exactly the wrong way around :P
		// It should be: symbol cache (by far the most likely location, also why it exists) -> PDB path in PE -> program directory.
		// (this is also the search order used by WinDbg/symchk/dumpbin and anything that uses symsrv)
		// WinDbg even tries HTTP servers before the path in the PE, but that might be taking it a bit too far

		// Symbol cache
		QDir cachePath(GlobalCfg.GetSymbolCacheDir());
		cachePath.cd(fileName);
		cachePath.cd(PdbSignature);
		cachePath.cd(fileName);
		PdbPaths.push_back(cachePath.absolutePath());

		// Debug directory full path
		const bool bAllowUncPathsInDebugDirectory =
			GlobalCfg.GetAllowUNCPathInDebugDirectory(); // TODO: create setting for this

		WCHAR szPdbFile[MAX_FILE_PATH_SIZE]; 
		PdbFile.toWCharArray(szPdbFile);
		if (qfPdbFile.isDir() && (bAllowUncPathsInDebugDirectory || !PathIsUNCW(szPdbFile)))
			PdbPaths.push_back(PdbFile);

		// Program directory (debug directory PDB name)
		PdbPaths.push_back(fileDir + QDir::separator() + fileName);

		// Program directory (file name PDB name)
		PdbPaths.push_back(fileDir + QDir::separator() + fileName + ".pdb");
	}
}

void Module::ReadExceptionDir()
{

}

quint64 Module::RVAToOffset(quint64 base, quint64 rva)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ImageNtHeaders);
	for (WORD i = 0; i < ImageNtHeaders->FileHeader.NumberOfSections; ++i)
	{
		if (rva >= section->VirtualAddress && rva < section->VirtualAddress + section->SizeOfRawData)
		{
			ASSERT_TRUE(rva != 0); // Following garbage in is garbage out, RVA 0 should always yield VA 0
			return base + (rva - section->VirtualAddress) + section->PointerToRawData;
		}
		section++;
	}
	return 0;
}

bool Module::IsDebugSupport(PIMAGE_DEBUG_DIRECTORY pEntry)
{
	// Check for valid RVA
	ULONG_PTR offset = 0;

	if (pEntry->AddressOfRawData)
		offset = (ULONG_PTR)RVAToOffset(0, pEntry->AddressOfRawData);
	else if (pEntry->PointerToRawData)
		offset = pEntry->PointerToRawData;
	if (!offset)
		return false;

	// Check size is sane and end of data lies within the image
	if (pEntry->SizeOfData < sizeof(CV_INFO_PDB20) /*smallest supported type*/ ||
		pEntry->AddressOfRawData + pEntry->SizeOfData > HEADER_FIELD(ImageNtHeaders, SizeOfImage))
		return false;

	// Choose from one of our many supported types such as codeview
	if (pEntry->Type == IMAGE_DEBUG_TYPE_CODEVIEW) // TODO: support other types (DBG)?
	{
		// Get the CV signature and do a final size check if it is valid
		auto signature = *(DWORD*)(MappingVA + offset);
		if (signature == '01BN')
		{
			return pEntry->SizeOfData >= sizeof(CV_INFO_PDB20) && pEntry->SizeOfData < sizeof(CV_INFO_PDB20) + DOS_MAX_PATH_LENGTH;
		}
		else if (signature == 'SDSR')
		{
			return pEntry->SizeOfData >= sizeof(CV_INFO_PDB70) && pEntry->SizeOfData < sizeof(CV_INFO_PDB70) + DOS_MAX_PATH_LENGTH;
		}
		else
		{
			qCritical("Unknown CodeView signature %08X for module %s%s...",
				signature,
				FileName.toUtf8().data(),
				FileExt.toUtf8().data());
			return false;
		}
	}
	return false;
}

QString Module::FormatDebugType(DWORD type)
{
	switch (type)
	{
	case IMAGE_DEBUG_TYPE_UNKNOWN:
		return "IMAGE_DEBUG_TYPE_UNKNOWN";
	case IMAGE_DEBUG_TYPE_COFF:
		return "IMAGE_DEBUG_TYPE_COFF";
	case IMAGE_DEBUG_TYPE_CODEVIEW:
		return "IMAGE_DEBUG_TYPE_CODEVIEW";
	case IMAGE_DEBUG_TYPE_FPO:
		return "IMAGE_DEBUG_TYPE_FPO";
	case IMAGE_DEBUG_TYPE_MISC:
		return "IMAGE_DEBUG_TYPE_MISC";
	case IMAGE_DEBUG_TYPE_EXCEPTION:
		return "IMAGE_DEBUG_TYPE_EXCEPTION";
	case IMAGE_DEBUG_TYPE_FIXUP:
		return "IMAGE_DEBUG_TYPE_FIXUP";
	case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
		return "IMAGE_DEBUG_TYPE_OMAP_TO_SRC";
	case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
		return "IMAGE_DEBUG_TYPE_OMAP_FROM_SRC";
	case IMAGE_DEBUG_TYPE_BORLAND:
		return "IMAGE_DEBUG_TYPE_BORLAND";
	case IMAGE_DEBUG_TYPE_RESERVED10:
		return "IMAGE_DEBUG_TYPE_RESERVED10";
	case IMAGE_DEBUG_TYPE_CLSID:
		return "IMAGE_DEBUG_TYPE_CLSID";
		// The following types aren't defined in older Windows SDKs, so just count up from here so we can still return the names for them
	case(IMAGE_DEBUG_TYPE_CLSID + 1):
		return "IMAGE_DEBUG_TYPE_VC_FEATURE"; // How to kill: /NOVCFEATURE linker switch
	case(IMAGE_DEBUG_TYPE_CLSID + 2):
		return "IMAGE_DEBUG_TYPE_POGO"; // How to kill: /NOCOFFGRPINFO linker switch
	case(IMAGE_DEBUG_TYPE_CLSID + 3):
		return "IMAGE_DEBUG_TYPE_ILTCG";
	case(IMAGE_DEBUG_TYPE_CLSID + 4):
		return "IMAGE_DEBUG_TYPE_MPX";
	case(IMAGE_DEBUG_TYPE_CLSID + 5):
		return "IMAGE_DEBUG_TYPE_REPRO";
	default:
		return "unknown";
	}
}

Module* Module::CreateModule(Process* process, LPVOID lpBaseOfDLL)
{
	TCHAR szMappedPath[MAX_FILE_PATH_SIZE] = { 0 };

	auto dllBase = lpBaseOfDLL;
	auto hProcess = process->Handle();
	auto pModule = new Module;

	pModule->ModBase = quint64(dllBase);

	// 得到Module的文件全路径
	if (!GetMappedFileName(hProcess, dllBase, szMappedPath, MAX_FILE_PATH_SIZE))
	{
		qWarning("GetMappedFileName failed.");
		return nullptr;
	}

	auto qsSystemDir = WinExtras::GetSystemDir();
	auto qsModName = WinExtras::TranslateNativeName(szMappedPath);
	qDebug("Load: %s", qsModName.toUtf8().data());

	pModule->Party = qsModName.contains(qsSystemDir, Qt::CaseInsensitive)
		? MODULE_PARTY::SYSTEM
		: MODULE_PARTY::USER;
	pModule->FilePath = qsModName;

	QFile qfModule(qsModName);
	QFileInfo qfi(qsModName);

	if (qsModName.contains("virtual:\\"))
	{
		// TODO: 
	}
	else
	{
		if (qfModule.open(QIODevice::ReadOnly))
		{
			pModule->ModSize = qfModule.size();
			pModule->MappingVA = (quint64)qfModule.map(0, pModule->ModSize);
		}
		else
		{
			pModule->ModSize = 0;
			pModule->MappingVA = 0;
		}
	}

	// Get the PE headers
	if (!NT_SUCCESS(pModule->InitImageNtHeader()))
	{
		auto name = qfi.baseName();
		auto ext = qfi.suffix();
		qCritical("Module %s%s: invalid PE file!", name.toUtf8().data(), ext.toUtf8().data());
		return nullptr;
	}

	// Enumerate all PE sections
	pModule->ListPESection();

	pModule->ListExports();
	pModule->ListImports();
	pModule->ListTLSCallbacks();
	pModule->ReadDebugDir();

	return pModule;
}
