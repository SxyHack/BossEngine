#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <QJsonObject>

class JsonMaker
{
public:
	// ��װ��QJsonObject
	static QJsonObject MakeWith(MODULEENTRY32& data);

	// ��ԭ
	static bool ReturnTo(const QJsonObject& json, MODULEENTRY32& data);
};