// header
#pragma once

#include <QObject>
#include <QList>
#include <Windows.h>
#include <tlhelp32.h>
#include <QtWin>

typedef QList<HWND>          LIST_HWND;
typedef QList<MODULEENTRY32> LIST_MODULE;
typedef QList<MEMORY_BASIC_INFORMATION> LIST_MEMORY;

//
// Windows API ��װ
//
class WinExtras : public QObject
{
	Q_OBJECT

public:
	WinExtras();
	~WinExtras();

	// 
	// ����Ȩ��
	// 
	BOOL AdjustPrivilege();
	BOOL EnableDebugPrivilege(BOOL bEnable = TRUE);
	//
	// ��ȡ���ڵľ��
	// dwPID: ����ID
	// ����ֵ: �����NULL, ˵��ʧ��
	//
	HWND QueryWindowHandle(DWORD dwPID);
	//
	// ��ȡ���̵�Ӧ��ͼ��
	//
	BOOL GetProcessICON(DWORD dwTargetPID, QPixmap& pixmap);
	//
	// GetLastError����ֵת���ַ���
	//
	static QString FormatLastError(DWORD lastErr);
	static QString FormatMemoryProtection(DWORD value);
	static QString FormatMemoryState(DWORD state);
	static QString FormatMemoryType(DWORD value);
	// 
	// ��ѯ���̵�ģ���б���Ϣ, ʹ��WindowsAPI
	// ����:
	// hProcess [IN] ���̾��
	//
	BOOL EnumProcessModulesNT(IN HANDLE hProcess);
	//
	// ��ѯ���̵�ģ���б���Ϣ, ʹ��ToolHelpLibrary
	// ����:
	// dwPID - �������ID
	//
	BOOL EnumProcessModulesTH(IN DWORD dwPID, OUT LIST_MODULE& modules);
	//
	// ö�ٽ��̵��ڴ���Ϣ, ��������һ����
	// IN hProcess: ���̾��
	// IN lpAddress: ��ʼ��ַ
	// OUT results: �������е��ڴ���Ϣ
	// 
	BOOL QueryProcessMemory(HANDLE hProcess, ULONG_PTR lpAddress, LIST_MEMORY& results);

	//
	// ��ȡϵͳĿ¼, C:\Windows\System
	//
	QString GetSystemDir();
private:


private:
	BOOLEAN _DebugPrivilege;
};
