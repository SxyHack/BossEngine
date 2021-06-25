#pragma once

#include <QThread>
#include <QSemaphore>

class Process;

class EnumModuleWorker : public QThread
{
	Q_OBJECT

public:
	EnumModuleWorker(Process* process);
	~EnumModuleWorker();

	void Stop();
	//
	// µÈ´ý³õÊ¼»¯
	// 
	void WaitForInit();

protected:
	void run() override;

private:
	Process*   _Process;
	QSemaphore _ExitSemaphore;
	QSemaphore _InitSemaphore;
};
