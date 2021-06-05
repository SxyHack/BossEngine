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
	// 设置输入值 A 和 B
	//
	void SetInputs(const QString& a, const QString& b);

	// 
	// 基础匹配数据, 第一次扫描的匹配方法.
	// 成功返回TRUE, 否则返回FALSE
	//
	virtual bool BaseMatch(BYTE* pBuf, SIZE_T nSize) = 0;
	virtual bool NextMatch(const QVariant& variant) = 0;

	//
	// 获取变量
	// 
	QVariant& GetValue();

	//
	// 创建比较策略的实例
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
