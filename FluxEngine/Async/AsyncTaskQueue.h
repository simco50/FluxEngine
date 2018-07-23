#pragma once
#include "Mutex.h"
#include "Core\Subsystem.h"

class WorkerThread;

struct AsyncTask
{
	unsigned Priority = 0;
	bool IsCompleted = false;
	SinglecastDelegate<void, AsyncTask*, unsigned> Action;
};

class AsyncTaskQueue : public Subsystem
{
	FLUX_OBJECT(AsyncTaskQueue, Subsystem)
	DELETE_COPY(AsyncTaskQueue)

public:
	AsyncTaskQueue(Context* pContext, const size_t count);
	~AsyncTaskQueue();

	void JoinAll();
	void ProcessItems(int index);
	AsyncTask* GetFreeTask();
	void AddWorkItem(AsyncTask* pItem);
	void Stop();
	size_t GetThreadCount() const { return m_Threads.size(); }

	void ParallelFor(const int count, const SinglecastDelegate<void, int>& function, bool singleThreaded = false);

	bool IsCompleted() const;

private:
	void CreateThreads(const size_t count);

	std::vector<std::unique_ptr<WorkerThread>> m_Threads;

	std::vector<std::unique_ptr<AsyncTask>> m_Tasks;
	std::vector<AsyncTask*> m_RunningTasks;
	std::vector<AsyncTask*> m_TaskPool;
	std::deque<AsyncTask*> m_Queue;

	Mutex m_QueueMutex;
	bool m_Shutdown = false;
	bool m_Paused = true;
};
