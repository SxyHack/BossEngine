#include "GlobalSettings.h"
#include <QtCore/QtCore>

GlobalSettings& GlobalSettings::Instance()
{
	static GlobalSettings instance;
	return instance;
}

void GlobalSettings::SetSymbolCacheDir(const QString& path)
{
	mSettings->beginGroup("MODULE");
	mSettings->setValue("symbol_cache_path", path);
	mSettings->sync();
	mSettings->endGroup();
}


QString GlobalSettings::GetSymbolCacheDir()
{
	mSettings->beginGroup("MODULE");
	auto dir = mSettings->value("symbol_cache_path", "").toString();
	mSettings->endGroup();
	return dir;
}

void GlobalSettings::SetAllowUnicodePathInDebugDirectory(bool allow)
{
	mSettings->beginGroup("MODULE");
	mSettings->setValue("allow_unicode_path_in_debug_dir", allow);
	mSettings->sync();
	mSettings->endGroup();
}

bool GlobalSettings::GetAllowUnicodePathInDebugDirectory()
{
	mSettings->beginGroup("MODULE");
	auto ret = mSettings->value("allow_unicode_path_in_debug_dir", false).toBool();
	mSettings->endGroup();
	return ret;
}

GlobalSettings::GlobalSettings()
	: mSettings(nullptr)
{
	auto sAppDir = QCoreApplication::applicationDirPath();
	QDir appDir(sAppDir);
	appDir.mkdir("config");
	appDir.cd("config");

	auto sPath = appDir.absolutePath() + "/global_settings.ini";
	mSettings = new QSettings(sPath, QSettings::IniFormat);
}

