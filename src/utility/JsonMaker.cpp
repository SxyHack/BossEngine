#include "JsonMaker.h"
#include <QJsonValue>

QJsonObject JsonMaker::MakeWith(MODULEENTRY32& data)
{
	QJsonObject jsonObject;
	jsonObject.insert("szExePath", QString::fromWCharArray(data.szExePath));
	jsonObject.insert("szModule", QString::fromWCharArray(data.szModule));
	jsonObject.insert("dwSize", (qint64)data.dwSize);
	jsonObject.insert("GlblcntUsage", (qint64)data.GlblcntUsage);
	jsonObject.insert("hModule", (qint64)data.hModule);
	jsonObject.insert("modBaseAddr", (qint64)data.modBaseAddr);
	jsonObject.insert("modBaseSize", (qint64)data.modBaseSize);
	jsonObject.insert("ProccntUsage", (qint64)data.ProccntUsage);
	jsonObject.insert("th32ModuleID", (qint64)data.th32ModuleID);
	jsonObject.insert("th32ProcessID", (qint64)data.th32ProcessID);

	return jsonObject;
}

bool JsonMaker::ReturnTo(const QJsonObject& json, MODULEENTRY32& data)
{
	ZeroMemory(&data, sizeof(MODULEENTRY32));

	if (json.isEmpty())
		return false;
	if (!json.contains("szExePath") ||
		!json.contains("szModule") ||
		!json.contains("dwSize") ||
		!json.contains("GlblcntUsage") ||
		!json.contains("hModule") ||
		!json.contains("modBaseAddr") ||
		!json.contains("modBaseSize") ||
		!json.contains("ProccntUsage") ||
		!json.contains("th32ModuleID") ||
		!json.contains("th32ProcessID"))
	{
		return false;
	}

	json["szExePath"].toString().toWCharArray(data.szExePath);
	json["szModule"].toString().toWCharArray(data.szModule);
	data.dwSize = json["dwSize"].toVariant().toLongLong();
	data.GlblcntUsage = json["GlblcntUsage"].toVariant().toLongLong();
	data.hModule = (HMODULE)json["hModule"].toVariant().toLongLong();
	data.modBaseAddr = (BYTE*)json["modBaseAddr"].toVariant().toLongLong();
	data.modBaseSize = (DWORD)json["modBaseSize"].toVariant().toLongLong();
	data.ProccntUsage = (DWORD)json["ProccntUsage"].toVariant().toLongLong();
	data.th32ModuleID = (DWORD)json["th32ModuleID"].toVariant().toLongLong();
	data.th32ProcessID = (DWORD)json["th32ProcessID"].toVariant().toLongLong();

	return true;
}

