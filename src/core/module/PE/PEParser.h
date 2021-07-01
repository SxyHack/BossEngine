#pragma once
#include <windows.h>
#include <ntstatus.h>
#include <minwindef.h>

class BEModule;

class PEParser
{
public:
	PEParser(BEModule* pModule);
	~PEParser();

	bool Execute();

private:
	NTSTATUS ParseDosHeader();
	NTSTATUS ParseNtHeader();
	NTSTATUS ParseSections();

private:
	BEModule* _Module;
};
