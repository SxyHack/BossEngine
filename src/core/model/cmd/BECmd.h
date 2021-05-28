#pragma once

#include <QThread>
#include <QList>

class BECmd : public QThread
{
	Q_OBJECT

public:
	BECmd();
	~BECmd();

	virtual void Stop() = 0;

signals:
	void ES_Done();
	void ES_Started();

};

typedef QList<BECmd*> LIST_CMD;