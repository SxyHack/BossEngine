#pragma once

#include <QThread>
#include <QList>

class BECmd : public QThread
{
	Q_OBJECT

public:
	BECmd();
	~BECmd();

	virtual qint64 GetFoundCount() = 0;
	virtual void Stop() = 0;

signals:
	void ES_Done(qint64);
	void ES_Started();

};

typedef QList<BECmd*> LIST_CMD;