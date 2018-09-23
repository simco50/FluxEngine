#pragma once

class AsyncTaskQueue;

class Thread
{
public:
	Thread();
	virtual ~Thread();

	DELETE_COPY(Thread)

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
	virtual ~WorkerThread() = default;

	DELETE_COPY(WorkerThread)

	virtual int ThreadFunction() override;
	bool Run();
	int GetIndex() const;
private:
	AsyncTaskQueue* m_pOwner;
	int m_Index;
};
