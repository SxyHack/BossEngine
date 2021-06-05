#include "BEValue.h"

BEPointer::BEPointer(EScanValueType type)
	: QObject(nullptr)
	, ValueType(type)
	, Index(0)
	, Address(0)
	, ModBaseAddr(0)
	, hProcess(NULL)
	, IsBaseScan(true)
	, ValueSize(0)
{
}

BEPointer::~BEPointer()
{
}

bool BEPointer::ReadMemory(QVariant& value)
{
	BYTE*  pBuffer = new BYTE[ValueSize];
	SIZE_T nNumberOfBytesRead = 0;

	ZeroMemory(pBuffer, ValueSize);

	if (!::ReadProcessMemory(hProcess, (LPVOID)Address, pBuffer, ValueSize, &nNumberOfBytesRead))
	{
		delete[] pBuffer;
		return false;
	}

	value = this->Value = MakeFrByte(ValueType, pBuffer, ValueSize);

	delete[] pBuffer;

	return this->Value.isValid();
}

bool BEPointer::ReadMemory()
{
	QVariant result;  // unused
	return ReadMemory(result);
}

bool BEPointer::operator==(const QVariant& input)
{
	return this->Value == input;
}

bool BEPointer::operator==(const QString& input)
{
	QVariant vInput = MakeFrString(ValueType, input);
	return this->Value == vInput;
}

QVariant BEPointer::MakeFrByte(EScanValueType valueType, BYTE* pBuffer, SIZE_T nSize)
{
	switch (valueType)
	{
	case EScanValueType_Byte:
		return QVariant(*pBuffer);
	case EScanValueType_2_Byte:
		return QVariant(*(WORD*)pBuffer);
	case EScanValueType_4_Byte:
		return QVariant(*(DWORD32*)pBuffer);
	case EScanValueType_8_Byte:
		return QVariant(*(DWORD64*)pBuffer);
	case EScanValueType_Float:
		return QVariant(*(FLOAT*)pBuffer);
	case EScanValueType_Double:
		return QVariant(*(DOUBLE*)pBuffer);
	case EScanValueType_String:
		return QVariant(QString::fromLocal8Bit((char*)pBuffer, nSize));
	case EScanValueType_All:
		qWarning("not support EScanValueType_All");
		break;
	}

	return QVariant(QVariant::Invalid);
}

QVariant BEPointer::MakeFrString(EScanValueType valueType, const QString& input)
{
	switch (valueType)
	{
	case EScanValueType_Byte:
		return QVariant((BYTE)input.toUInt());
	case EScanValueType_2_Byte:
		return QVariant((WORD)input.toUInt());
	case EScanValueType_4_Byte:
		return QVariant((DWORD32)input.toUInt());
	case EScanValueType_8_Byte:
		return QVariant((DWORD64)input.toULongLong());
	case EScanValueType_Float:
		return QVariant((FLOAT)input.toFloat());
	case EScanValueType_Double:
		return QVariant((DOUBLE)input.toDouble());
	case EScanValueType_String:
		return QVariant(input);
	case EScanValueType_All:
		qWarning("not support EScanValueType_All");
		break;
	}

	return QVariant(QVariant::Invalid);
}
