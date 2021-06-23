#pragma once

#include <QPair>
#include <QString>
#include <Windows.h>

#define MAX_MODULE_SIZE 1024

enum class MODULE_PARTY
{
	USER,
	SYSTEM
};

struct CV_HEADER
{
	DWORD Signature;
	DWORD Offset;
};

struct CV_INFO_PDB20
{
	CV_HEADER CvHeader; //CvHeader.Signature = "NB10"
	DWORD Signature;
	DWORD Age;
	BYTE PdbFileName[1];
};

struct CV_INFO_PDB70
{
	DWORD CvSignature; //"RSDS"
	GUID Signature;
	DWORD Age;
	BYTE PdbFileName[1];
};

struct PDB_VALIDATION
{
	GUID Guid;
	DWORD Signature = 0;
	DWORD Age = 0;

	PDB_VALIDATION()
	{
		memset(&Guid, 0, sizeof(GUID));
	}
};


class QRange : public QPair<quint64, quint64>
{
public:
	QRange(quint64 start, quint64 size) : QPair(start, size) {}

	bool operator==(const QRange& b)
	{
		return this->second < b.first;
	}
};

