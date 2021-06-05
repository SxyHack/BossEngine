#pragma once

#include <QObject>
#include <QMutex>

#include "WinExtras.h"
#include "core/model/BEWorkspace.h"

class BEngine : public QObject
{
	Q_OBJECT

public:
	~BEngine();

	static BEngine& Instance();

	//
	// ö�����н���, �ᷢ�������ź� sgEnumProcessPrepare, sgEnumProcess, sgEnumProcessDone
	// sgEnumProcessPrepare: ��ѭ����ʼǰ����
	// sgEnumProcess: ÿ����һ������ʱ����
	// sgEnumProcessDone: ����ѭ����������
	//
	void EnumProcess(const QString& filter = QString());

	//
	// �򿪽���, ׼��ɨ����߶�д
	// 
	BOOL OpenProcess(DWORD pid);
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
	void sgEnumProcessPrepare();
	void sgEnumProcess(qint32, PROCESSENTRY32);
	void sgEnumProcessDone(qint32);

private:
	BEngine();

	WinExtras      _WinExtras;
	DWORD          _AttachProcessID;
	HANDLE         _AttachProcessHandle;
	LIST_MODULE    _AttachProcessModules;
	LIST_MODULE    _IncludeModules;
	DWORD          _LastErrorCode;
	QString        _LastErrorMessage;
	LIST_WORKSPACE _Workspaces;
};

#define Engine BEngine::Instance()
