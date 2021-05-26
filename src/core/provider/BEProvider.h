#pragma once

#include <QObject>
#include "WinExtras.h"

//
// 提供器基类
//
class BEProvider : public QObject
{
	Q_OBJECT

public:
	BEProvider(QObject *parent);
	~BEProvider();

protected:
	WinExtras _WinExtras;
};
