#pragma once

#include <QObject>
#include "WinExtras.h"

//
// �ṩ������
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
