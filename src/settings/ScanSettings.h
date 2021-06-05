#pragma once
#include <QSettings>
#include <Windows.h>

class ScanSettings
{
public:
	~ScanSettings();
	static ScanSettings& Instance();

	// + MEM_TYPE
	void SetMemMappedEnable(bool enable);
	bool GetMemMappedEnable();

	void SetMemImageEnable(bool enable);
	bool GetMemImageEnable();

	void SetMemPrivateEnable(bool enable);
	bool GetMemPrivateEnable();
	// - MEM_TYPE

	// + ������������
	void SetAllTypeIncludeByte(bool include);
	bool GetAllTypeIncludeByte();

	void SetAllTypeInclude2Byte(bool include);
	bool GetAllTypeInclude2Byte();

	void SetAllTypeInclude4Byte(bool include);
	bool GetAllTypeInclude4Byte();

	void SetAllTypeInclude8Byte(bool include);
	bool GetAllTypeInclude8Byte();

	void SetAllTypeIncludeFloat(bool include);
	bool GetAllTypeIncludeFloat();

	void SetAllTypeIncludeDouble(bool include);
	bool GetAllTypeIncludeDouble();
	// - ������������

	// + PAGE_PROTECT
	// �����Ƿ�ɨ�� PAGE_GUARD �����ڴ�ҳ��, Ĭ�ϲ�ɨ��
	void SetPageGuardEnable(bool enable);
	bool GetPageGuardEnable();

	// PAGE_NOACCESS 
	void SetPageNoAccessEnable(bool enable);
	bool GetPageNoAccessEnable();

	// PAGE_NOCACHE
	void SetPageNoCacheEnable(bool enable);
	bool GetPageNoCacheEnable();

	// PAGE_WRITECOMBINE
	void SetPageWriteCombineEnable(bool enable);
	bool GetPageWriteCombineEnable();

protected:
	ScanSettings();

private:
	QSettings* _Settings;
};


#define SCAN_SETTING ScanSettings::Instance()