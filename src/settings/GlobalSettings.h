#pragma once

#include <QSettings>

class GlobalSettings
{
public:
	static GlobalSettings& Instance();

	void SetSymbolCacheDir(const QString& path);
	QString GetSymbolCacheDir();

	void SetAllowUnicodePathInDebugDirectory(bool allow);
	bool GetAllowUnicodePathInDebugDirectory();

protected:
	GlobalSettings();

private:
	QSettings* mSettings;
};

#define GlobalCfg GlobalSettings::Instance()
