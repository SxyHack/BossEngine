#pragma once

#include <QObject>
#include <QMutex>

#include "WinExtras.h"
#include "core/model/BEWorkspace.h"
#include "Process.h"
#include "Debugger.h"

class BEngine : public QObject
{
	Q_OBJECT

public:
	~BEngine();

	static BEngine& Instance();

	//
	// ö�����н���, �ᷢ�������ź� sgListProcessStart, sgListProcess, sgListProcessDone
	// sgListProcessStart: ��ѭ����ʼǰ����
	// sgListProcess: ÿ����һ������ʱ����
	// sgListProcessDone: ����ѭ����������
	//
	void ListProcesses(const QString& filter = QString());

public:
	//
	// �򿪽���, ׼��ɨ����߶�д
	// 
	BOOL OpenProcess(DWORD pid);
	void CloseProcess();
	HANDLE GetProcessHandle();

	//////////////////////////////////////////////////////////////////////////
	// 
	// ö�ٴ򿪽��̵�����ģ��, ��Ҫ�ȵ��� OpenProcess.
	//
	BOOL EnumModules();
	LIST_MODULE& GetModules();
	//
	// ��ģ����ӵ�_IncludeModules����, ���������������߳���ʹ��
	//
	void IncludeModule(const MODULEENTRY32& mod);
	//
	// ��� _IncludeModules ����
	//
	void RemoveIncludeModules();
	LIST_MODULE& GetIncludeModules();

	//
	// ��ѯ��̬��ַ
	// 	   ulAddress: �����ַ
	// 	   modName: ����ģ������
	// return (qint64): ����ƫ��
	//
	qint64 QueryStaticAddress(quint64 ulAddress, QString& modName);
	//////////////////////////////////////////////////////////////////////////
	//
	// ��ȡ����ģ������ֽ���
	//
	DWORD64 GetModulesSize();
	//
	// ö�ٽ��̵��ڴ��
	//
	BOOL EnumVirtualMemory();

	//
	// ��ȡGetLastError����ֵ
	//
	DWORD GetLastErrorCode();
	QString GetLastErrorMessage();

	//////////////////////////////////////////////////////////////////////////
	// �����ռ�
#pragma region WORKSPACE
	BEWorkspace* AddWorkspace(qint32 uniqueID, const QString& name=QString());
#pragma endregion


signals:
	void sgListProcessStart();
	void sgListProcess(qint32, PROCESSENTRY32);
	void sgListProcessDone(qint32);

private:
	BEngine();

	WinExtras      _WinExtras;
	DWORD          _AttachProcessID;
	HANDLE         _AttachProcessHandle;
	Process        _AttachProcess;
	LIST_MODULE    _AttachProcessModules;
	LIST_MODULE    _IncludeModules;
	DWORD          _LastErrorCode;
	QString        _LastErrorMessage;
	LIST_WORKSPACE _Workspaces;
};

#define Engine BEngine::Instance()
