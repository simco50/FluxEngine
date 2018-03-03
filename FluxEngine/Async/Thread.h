#pragma once
#include "AsyncTaskQueue.h"

class AsyncTaskQueue;

class Thread
{
public:
	Thread();
	~Thread();

	bool Run();
	void Stop();
	bool SetPriority(const int priority);

	virtual int ThreadFunction() = 0;

	//Get the given thread ID
	unsigned long GetId() const { return m_ThreadId; }
	bool IsCurrentThread() const { return GetId() == GetCurrentId(); }
	static unsigned int GetCurrentId();

	static void SetMainThread();
	static bool IsMainThread();

private:
	static DWORD WINAPI ThreadFunctionStatic(void* pData);
	static unsigned int m_MainThread;
	unsigned long m_ThreadId;
	void* m_pHandle = nullptr;
};

class WorkerThread : public Thread
{
public:
	WorkerThread(AsyncTaskQueue* pOwner, int index);
	virtual int ThreadFunction() override;

	int GetIndex() const { return m_Index; }
private:
	AsyncTaskQueue* m_pOwner;
	int m_Index;
};
