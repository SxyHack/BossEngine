#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <QJsonObject>

class JsonMaker
{
public:
	// 封装成QJsonObject
	static QJsonObject MakeWith(MODULEENTRY32& data);

	// 还原
	static bool ReturnTo(const QJsonObject& json, MODULEENTRY32& data);
};