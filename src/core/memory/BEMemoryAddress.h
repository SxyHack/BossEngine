#pragma once

#include <QObject>

class BEMemoryAddress : public QObject
{
	Q_OBJECT

public:
	BEMemoryAddress(QObject *parent);
	~BEMemoryAddress();
};
