#pragma once

#include "BECompare.h"

class ExactCompare : public BECompare
{
	Q_OBJECT

public:
	ExactCompare(EScanValueType valueType);
	~ExactCompare();

	bool BaseMatch(BYTE* pBuf, SIZE_T nSize) override;
	bool NextMatch(const QVariant& variant) override;
};
