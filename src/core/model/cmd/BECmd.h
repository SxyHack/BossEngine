#pragma once

#include <QObject>
#include <QThread>

class BECmd : public QObject, public QThread
{
	Q_OBJECT

public:
	BECmd();
	~BECmd();

	void run() override;

protected:

};
