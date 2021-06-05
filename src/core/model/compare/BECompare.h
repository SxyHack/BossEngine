#pragma once

#include <QObject>
#include "BEValue.h"

class BECompare : public QObject
{
	Q_OBJECT

public:
	BECompare(EScanValueType valueType);
	~BECompare();

	//
	// ��������ֵ A �� B
	//
	void SetInputs(const QString& a, const QString& b);

	// 
	// ����ƥ������, ��һ��ɨ���ƥ�䷽��.
	// �ɹ�����TRUE, ���򷵻�FALSE
	//
	virtual bool BaseMatch(BYTE* pBuf, SIZE_T nSize) = 0;
	virtual bool NextMatch(const QVariant& variant) = 0;

	//
	// ��ȡ����
	// 
	QVariant& GetValue();

	//
	// �����Ƚϲ��Ե�ʵ��
	//
	static BECompare* Create(ECompareMethod method, EScanValueType vt);

protected:
	QVariant MakeFrString(const QString& input);
	QVariant MakeFrByte(BYTE* pBuffer, SIZE_T nSize);

protected:
	qint64         _ValueSize;
	EScanValueType _ValueType;
	QVariant       _Value;
	QVariant       _InputA;
	QVariant       _InputB;
	bool           _IsBase;
};
