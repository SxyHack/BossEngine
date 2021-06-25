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
	// �ȴ���ʼ��
	// 
	void WaitForInit();

protected:
	void run() override;

private:
	Process*   _Process;
	QSemaphore _ExitSemaphore;
	QSemaphore _InitSemaphore;
};
