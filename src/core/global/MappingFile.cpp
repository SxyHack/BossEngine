#include "MappingFile.h"


MappingFile::MappingFile(const QString& path, Access access)
	: QObject(nullptr)
	, _DesiredAccess_(0)
	, _FilePath_(path)
	, _Access_(access)
{
	DWORD FileAccess = 0;
	DWORD FileMapType = 0;
	DWORD FileMapViewType = 0;

	switch (_Access_)
	{
	case MappingFile::Access::ACCESS_READ:
		FileAccess = GENERIC_READ;
		FileMapType = PAGE_READONLY;
		FileMapViewType = FILE_MAP_READ;
		break;
	case MappingFile::Access::ACCESS_WRITE:
		FileAccess = GENERIC_WRITE;
		FileMapType = PAGE_READWRITE;
		FileMapViewType = FILE_MAP_WRITE;
		break;
	case MappingFile::Access::ACCESS_ALL:
		FileAccess = GENERIC_READ + GENERIC_WRITE + GENERIC_EXECUTE;
		FileMapType = PAGE_EXECUTE_READWRITE;
		FileMapViewType = FILE_MAP_WRITE;
		break;
	default:
		FileAccess = GENERIC_READ + GENERIC_WRITE + GENERIC_EXECUTE;
		FileMapType = PAGE_EXECUTE_READWRITE;
		FileMapViewType = FILE_MAP_ALL_ACCESS;
		break;
	}

	TCHAR szFileName[1024];
	path.toWCharArray(szFileName);

	_File_.handle();

	//HANDLE hFile = CreateFile(szFileName, FileAccess, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (hFile != INVALID_HANDLE_VALUE)
	//{
	//	*FileHandle = hFile;
	//	DWORD mfFileSize = GetFileSize(hFile, NULL);
	//	mfFileSize = mfFileSize + SizeModifier;
	//	*FileSize = mfFileSize;
	//	HANDLE mfFileMap = CreateFileMappingA(hFile, NULL, FileMapType, NULL, mfFileSize, NULL);
	//	if (mfFileMap != NULL)
	//	{
	//		*FileMap = mfFileMap;
	//		LPVOID mfFileMapVA = MapViewOfFile(mfFileMap, FileMapViewType, NULL, NULL, NULL);
	//		if (mfFileMapVA != NULL)
	//		{
	//			RtlMoveMemory(FileMapVA, &mfFileMapVA, sizeof ULONG_PTR);
	//			return true;
	//		}
	//	}
	//	RtlZeroMemory(FileMapVA, sizeof ULONG_PTR);
	//	*FileHandle = NULL;
	//	*FileSize = NULL;
	//	EngineCloseHandle(hFile);
	//}
	//else
	//{
	//	RtlZeroMemory(FileMapVA, sizeof ULONG_PTR);
	//}
	//return false;
}

MappingFile::~MappingFile()
{
}
