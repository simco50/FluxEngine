#pragma once

class AsyncTaskQueue;

class Thread
{
public:
	Thread();
	virtual ~Thread();

	bool SetPriority(const int priority);

	//Get the given thread ID
	unsigned long GetId() const { return m_ThreadId; }
	bool IsCurrentThread() const { return GetId() == GetCurrentId(); }
	static unsigned int GetCurrentId();
	bool IsRunning() const { return m_pHandle != nullptr; }

	static void SetMainThread();
	static bool IsMainThread();
	static bool IsMainThread(unsigned int id);

protected:
	bool RunThread();
	void StopThread();

private:
	virtual int ThreadFunction() = 0;
	static DWORD WINAPI ThreadFunctionStatic(void* pData);
	static unsigned int m_MainThread;
	unsigned long m_ThreadId = 0;
	void* m_pHandle = nullptr;
};

class WorkerThread : public Thread
{
public:
	WorkerThread(AsyncTaskQueue* pOwner, int index);
	virtual int ThreadFunction() override;

	bool Run() { return RunThread(); }

	int GetIndex() const { return m_Index; }
private:
	AsyncTaskQueue* m_pOwner;
	int m_Index;
};
