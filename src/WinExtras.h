// header
#pragma once

#include <QObject>
#include <QList>
#include <Windows.h>
#include <tlhelp32.h>
#include <QtWin>

typedef QList<HWND>          LIST_HWND;
typedef QList<MODULEENTRY32> LIST_MODULE;

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
	QString FormatLastError(DWORD lastErr);
	// 
	// ��ѯ���̵�ģ���б���Ϣ, ʹ��WindowsAPI
	// ����:
	// hProcess [IN] ���̾��
	//
	BOOL QueryProcessModules(IN HANDLE hProcess);
	//
	// ��ѯ���̵�ģ���б���Ϣ, ʹ��ToolHelpLibrary
	// ����:
	// dwPID - �������ID
	//
	BOOL QueryProcessModulesTH(IN DWORD dwPID, OUT LIST_MODULE& modules);
private:


private:
	BOOLEAN _DebugPrivilege;
};
