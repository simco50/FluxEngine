#pragma once

class AsyncTaskQueue;

class Thread
{
public:
	using ThreadFunction = DWORD(*)(void*);

	Thread();
	virtual ~Thread();

	DELETE_COPY(Thread)

	bool SetPriority(const int priority);
	void SetAffinity(const uint64 affinity);
	void LockToCore(const uint32 core);

	static void SetCurrentAffinity(const uint64 affinity);
	static void LockCurrentToCore(const uint32 core);

	//Get the given thread ID
	unsigned long GetId() const { return m_ThreadId; }
	bool IsCurrentThread() const { return GetId() == GetCurrentId(); }
	static uint32 GetCurrentId();
	bool IsRunning() const { return m_pHandle != nullptr; }

	static void SetMainThread();
	static bool IsMainThread();
	static bool IsMainThread(uint32 id);
	static void SetName(uint32 id, const std::string& name);

	bool RunThread(ThreadFunction function, void* pArgs);
	void StopThread();

private:
	static void SetAffinity(void* pHandle, const uint64 affinity);

	static unsigned int m_MainThread;
	uint32 m_ThreadId = 0;
	void* m_pHandle = nullptr;
};

class HookableThread : public Thread
{
public:
	bool RunThread();
protected:
	virtual int ThreadFunction() = 0;
	static DWORD WINAPI ThreadFunctionStatic(void* pData);
};

class WorkerThread : public HookableThread
{
public:
	WorkerThread(AsyncTaskQueue* pOwner, int index);
	virtual ~WorkerThread() = default;

	DELETE_COPY(WorkerThread)

	virtual int ThreadFunction() override;
	int GetIndex() const;
private:
	AsyncTaskQueue* m_pOwner;
	int m_Index;
};
