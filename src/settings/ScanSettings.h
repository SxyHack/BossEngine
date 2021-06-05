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

	// + 所有类型配置
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
	// - 所有类型配置

	// + PAGE_PROTECT
	// 设置是否扫描 PAGE_GUARD 属性内存页面, 默认不扫描
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