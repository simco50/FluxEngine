#pragma once
#include "Mutex.h"
#include "Core\Subsystem.h"

class WorkerThread;
struct AsyncTask;

DECLARE_DELEGATE(AsyncTaskDelegate, AsyncTask*, unsigned int);
DECLARE_DELEGATE(ParallelForDelegate, int);

struct AsyncTask
{
	unsigned Priority = 0;
	bool IsCompleted = false;
	AsyncTaskDelegate Action;
};

class AsyncTaskQueue : public Subsystem
{
	FLUX_OBJECT(AsyncTaskQueue, Subsystem)
	DELETE_COPY(AsyncTaskQueue)

public:
	AsyncTaskQueue(Context* pContext, size_t count);
	~AsyncTaskQueue();

	void JoinAll();
	void ProcessItems(int index);
	void AddWorkItem(const AsyncTaskDelegate& action, int priority = 0);
	void Stop();
	size_t GetThreadCount() const { return m_Threads.size(); }

	void ParallelFor(int count, const ParallelForDelegate& function, bool singleThreaded = false);

	bool IsCompleted() const;

private:
	AsyncTask* GetFreeTask();
	void PreAllocateJobs(size_t count);
	void CreateThreads(size_t count);

	std::vector<std::unique_ptr<WorkerThread>> m_Threads;

	//Owner of all the tasks
	std::vector<std::unique_ptr<AsyncTask>> m_Tasks;
	//List of running tasks
	std::vector<AsyncTask*> m_RunningTasks;
	//List of free tasks
	std::vector<AsyncTask*> m_TaskPool;
	//Queued tasks
	std::deque<AsyncTask*> m_Queue;

	Mutex m_QueueMutex;
	bool m_Shutdown = false;
	bool m_Paused = true;
};
