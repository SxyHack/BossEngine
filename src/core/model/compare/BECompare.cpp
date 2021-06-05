#include "BECompare.h"
#include "ExactCompare.h"

BECompare::BECompare(EScanValueType valueType) : QObject(nullptr)
	, _ValueType(valueType)
	, _ValueSize(GetScanValueTypeSize(valueType))
{
}

BECompare::~BECompare()
{
}

//void BECompare::SetValueType(EScanValueType type)
//{
//	_ValueType = type;
//	_ValueSize = GetScanValueTypeSize(type);
//}

void BECompare::SetInputs(const QString& a, const QString& b)
{
	_InputA = MakeFrString(a);
	_InputB = MakeFrString(b);
	_IsBase = true;
}

QVariant& BECompare::GetValue()
{
	return _Value;
}

BECompare* BECompare::Create(ECompareMethod method, EScanValueType vt)
{

	switch (method)
	{
	case ECompareMethod_Exact:
		return new ExactCompare(vt);
	case ECompareMethod_MoreThan:
		break;
	case ECompareMethod_LessThan:
		break;
	case ECompareMethod_Between:
		break;
	case ECompareMethod_Unknown:
		break;
	case ECompareMethod_Bigger:
		break;
	case ECompareMethod_Smaller:
		break;
	case ECompareMethod_IncreaseN:
		break;
	case ECompareMethod_DecreaseN:
		break;
	case ECompareMethod_Changed:
		break;
	case ECompareMethod_Unchanged:
		break;
	case ECompareMethod_EqualBase:
		break;
	default:
		break;
	}

	return nullptr;
}

QVariant BECompare::MakeFrString(const QString& input)
{
	if (_ValueSize == 0)
	{
		qWarning("Must be call SetValueType before");
		return QVariant();
	}

	switch (_ValueType)
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

	return QVariant();
}

QVariant BECompare::MakeFrByte(BYTE* pBuffer, SIZE_T nSize)
{
	switch (_ValueType)
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

	return QVariant();
}
