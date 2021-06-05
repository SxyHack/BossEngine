#include "ScanSettings.h"
#include <QtCore/QtCore>

ScanSettings::ScanSettings()
	: _Settings(nullptr)
{
	auto sAppDir = QCoreApplication::applicationDirPath();
	QDir appDir(sAppDir);
	appDir.mkdir("config");
	appDir.cd("config");

	auto sPath = appDir.absolutePath() + "/scan.ini";
	_Settings = new QSettings(sPath, QSettings::IniFormat);
}

ScanSettings::~ScanSettings()
{
}

ScanSettings& ScanSettings::Instance()
{
	static ScanSettings instance;
	return instance;
}

void ScanSettings::SetMemMappedEnable(bool enable)
{
	_Settings->setValue("ENABLE_MEM_MAPPED", enable);
	_Settings->sync();
}

void ScanSettings::SetMemImageEnable(bool enable)
{
	_Settings->setValue("ENABLE_MEM_IMAGE", enable);
	_Settings->sync();
}

void ScanSettings::SetMemPrivateEnable(bool enable)
{
	_Settings->setValue("ENABLE_MEM_PRIVATE", enable);
	_Settings->sync();
}

bool ScanSettings::GetMemMappedEnable()
{
	return _Settings->value("ENABLE_MEM_MAPPED", false).toBool();
}

bool ScanSettings::GetMemImageEnable()
{
	return _Settings->value("ENABLE_MEM_IMAGE", false).toBool();
}

bool ScanSettings::GetMemPrivateEnable()
{
	return _Settings->value("ENABLE_MEM_PRIVATE", true).toBool();
}

void ScanSettings::SetAllTypeIncludeByte(bool include)
{
	_Settings->setValue("INCLUDE_BYTE", include);
	_Settings->sync();
}

bool ScanSettings::GetAllTypeIncludeByte()
{
	return _Settings->value("INCLUDE_BYTE", false).toBool();
}

void ScanSettings::SetAllTypeInclude2Byte(bool include)
{
	_Settings->setValue("INCLUDE_BYTE_2", include);
	_Settings->sync();
}

bool ScanSettings::GetAllTypeInclude2Byte()
{
	return _Settings->value("INCLUDE_BYTE_2", false).toBool();
}

void ScanSettings::SetAllTypeInclude4Byte(bool include)
{
	_Settings->setValue("INCLUDE_BYTE_4", include);
	_Settings->sync();
}

bool ScanSettings::GetAllTypeInclude4Byte()
{
	return _Settings->value("INCLUDE_BYTE_4", true).toBool();
}

void ScanSettings::SetAllTypeInclude8Byte(bool include)
{
	_Settings->setValue("INCLUDE_BYTE_8", include);
	_Settings->sync();
}

bool ScanSettings::GetAllTypeInclude8Byte()
{
	return _Settings->value("INCLUDE_BYTE_8", false).toBool();
}

void ScanSettings::SetAllTypeIncludeFloat(bool include)
{
	_Settings->setValue("INCLUDE_FLOAT", include);
	_Settings->sync();
}

bool ScanSettings::GetAllTypeIncludeFloat()
{
	return _Settings->value("INCLUDE_FLOAT", true).toBool();
}

void ScanSettings::SetAllTypeIncludeDouble(bool include)
{
	_Settings->setValue("INCLUDE_DOUBLE", include);
	_Settings->sync();
}

bool ScanSettings::GetAllTypeIncludeDouble()
{
	return _Settings->value("INCLUDE_DOUBLE", true).toBool();
}

void ScanSettings::SetPageGuardEnable(bool enable)
{
	_Settings->setValue("ENABLE_PAGE_GUARD", enable);
	_Settings->sync();
}

bool ScanSettings::GetPageGuardEnable()
{
	return _Settings->value("ENABLE_PAGE_GUARD", true).toBool();
}

void ScanSettings::SetPageNoAccessEnable(bool enable)
{
	_Settings->setValue("ENABLE_PAGE_NOACCESS", enable);
	_Settings->sync();
}

bool ScanSettings::GetPageNoAccessEnable()
{
	return _Settings->value("ENABLE_PAGE_NOACCESS", false).toBool();
}

void ScanSettings::SetPageNoCacheEnable(bool enable)
{
	_Settings->setValue("ENABLE_PAGE_NOCACHE", enable);
	_Settings->sync();
}

bool ScanSettings::GetPageNoCacheEnable()
{
	return _Settings->value("ENABLE_PAGE_NOCACHE", false).toBool();
}

void ScanSettings::SetPageWriteCombineEnable(bool enable)
{
	_Settings->setValue("ENABLE_PAGE_WRITECOMBINE", enable);
	_Settings->sync();
}

bool ScanSettings::GetPageWriteCombineEnable()
{
	return _Settings->value("ENABLE_PAGE_WRITECOMBINE", false).toBool();
}
