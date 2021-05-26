#pragma once

#include <Windows.h>
#include <winternl.h>

#pragma comment(lib,"ntdll.lib")

typedef _CLIENT_ID* PCLIENT_ID;

// link: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-adts/0a1ecc7f-1e7b-4f2c-9fdd-232c3c029338
// Specifies the privilege to manage auditing and the security log.
#define SE_SECURITY_PRIVILEGE 0x00000008

// Specifies the privilege to take ownership of objects. 
// Possession of this privilege overrides the DACL on an object and gives the possessor implicit RIGHT_WRITE_OWNER access.
#define SE_TAKE_OWNERSHIP_PRIVILEGE 0x00000009

// Specifies the privilege to restore objects.
#define SE_RESTORE_PRIVILEGE 0x00000012

// Specifies the privilege to debug the system.
#define SE_DEBUG_PRIVILEGE 0x00000014

// Specifies the privilege to enable accounts to be trusted for delegation.
#define SE_ENABLE_DELEGATION_PRIVILEGE 0x0000001B

// Ã·…˝»®œﬁ
EXTERN_C NTSYSAPI NTSTATUS NTAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

EXTERN_C NTSYSAPI NTSTATUS NTAPI NtOpenProcess(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);

EXTERN_C NTSYSAPI PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader(PVOID);

//EXTERN_C NTSYSAPI NTSTATUS NTAPI NtQueryInformationProcess(
//	IN  HANDLE ProcessHandle,
//	IN  PROCESS_INFORMATION_CLASS ProcessInformationClass,
//	OUT PVOID  ProcessInformation,
//	IN  ULONG  ProcessInformationLength,
//	OUT PULONG ReturnLength);

EXTERN_C NTSTATUS NTAPI NtWriteVirtualMemory(HANDLE, PVOID, PVOID, ULONG, PULONG);
