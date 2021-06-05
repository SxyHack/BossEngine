#include "ExactCompare.h"

ExactCompare::ExactCompare(EScanValueType valueType) : BECompare(valueType)
{
}

ExactCompare::~ExactCompare()
{
}

bool ExactCompare::BaseMatch(BYTE* pBuf, SIZE_T nSize)
{
	_Value = MakeFrByte(pBuf, nSize);
	return _Value == _InputA;
}

bool ExactCompare::NextMatch(const QVariant& variant)
{
	return variant == _InputA;
}
