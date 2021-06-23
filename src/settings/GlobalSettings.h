#pragma once

#include <QSettings>

class GlobalSettings
{
public:
	static GlobalSettings& Instance();

	void SetSymbolCacheDir(const QString& path);
	QString GetSymbolCacheDir();

	void SetAllowUNCPathInDebugDirectory(bool allow);
	bool GetAllowUNCPathInDebugDirectory();

protected:
	GlobalSettings();

private:
	QSettings* mSettings;
};

#define GlobalCfg GlobalSettings::Instance()
