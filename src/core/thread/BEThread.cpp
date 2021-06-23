#include "BEThread.h"

BEThread::BEThread(THREADENTRY32& tlh32)
	: QObject(nullptr)
	, Tlh32Entry(tlh32)
	, ThreadID(tlh32.th32ThreadID)
{
}

BEThread::~BEThread()
{
}
