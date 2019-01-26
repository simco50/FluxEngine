#include "FluxEngine.h"
#include "AsyncTaskQueue.h"
#include "Thread.h"

AsyncTaskQueue::AsyncTaskQueue(Context* pContext)
	: Subsystem(pContext)
{


}

AsyncTaskQueue::~AsyncTaskQueue()
{
	m_Shutdown = true;

	m_Tasks.clear();
	m_Threads.clear();
}

void AsyncTaskQueue::Initialize(uint32 threads)
{
	AUTOPROFILE(AsyncTaskQueue_Initialize);
#ifdef THREADING
	CreateThreads(threads);
	PreAllocateJobs(100);
#else
	count;
#endif
}

void AsyncTaskQueue::PreAllocateJobs(size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		std::unique_ptr<AsyncTask> pTask = std::make_unique<AsyncTask>();
		m_Tasks.push_back(std::move(pTask));
	}
}

void AsyncTaskQueue::CreateThreads(size_t count)
{
	for (size_t i = m_Threads.size(); i < count; ++i)
	{
		std::unique_ptr<WorkerThread> pThread = std::make_unique<WorkerThread>(this, (int)i);
		pThread->RunThread();
		m_Threads.push_back(std::move(pThread));
	}
}

void AsyncTaskQueue::JoinAll()
{
	m_Paused = false;
#ifdef THREADING
	for (;;)
	{
		if (IsCompleted())
		{
			break;
		}
	}
#else
	//Execute all of them on the main thread (current)
	for (AsyncTask* pTask : m_Queue)
	{
		pTask->Action.ExecuteIfBound(pTask, 0);
	}
	m_Queue.clear();
#endif

	for (auto& pTask : m_RunningTasks)
	{
		pTask->Action.Clear();
		pTask->IsCompleted = false;
		pTask->Priority = 0;
		m_TaskPool.push_back(pTask);
	}
	m_RunningTasks.clear();
}

void AsyncTaskQueue::ProcessItems(int index)
{
	bool wasActive = false;

	for (;;)
	{
		if (m_Shutdown)
		{
			return;
		}

		if (m_Paused && !wasActive)
		{
			Sleep(0);
		}

		m_QueueMutex.Lock();
		if (!m_Queue.empty())
		{
			wasActive = true;

			AsyncTask* pItem = m_Queue.front();
			m_Queue.pop_front();
			m_QueueMutex.Unlock();

			pItem->Action.ExecuteIfBound(pItem, index);
			pItem->IsCompleted = true;
		}
		else
		{
			wasActive = false;
			m_QueueMutex.Unlock();
			Sleep(0);
		}
	}
}

AsyncTask* AsyncTaskQueue::GetFreeTask()
{
	assert(Thread::IsMainThread());

	if (m_TaskPool.size() > 0)
	{
		AsyncTask* pTask = m_TaskPool.back();
		m_TaskPool.pop_back();
		return pTask;
	}
	else
	{
		std::unique_ptr<AsyncTask> pTask = std::make_unique<AsyncTask>();
		m_Tasks.push_back(std::move(pTask));
		return m_Tasks.back().get();
	}
}

void AsyncTaskQueue::AddWorkItem(const AsyncTaskDelegate& action, int priority /*= 0*/)
{
	AsyncTask* pTask = GetFreeTask();
	pTask->Action = action;
	pTask->Priority = priority;

	checkf(std::find_if(m_TaskPool.begin(), m_TaskPool.end(), [pTask](AsyncTask* pOther) {return pOther == pTask; }) == m_TaskPool.end(), "[AsyncTaskQueue::AddWorkItem] > Task is still in the pool");

	m_RunningTasks.push_back(pTask);

	ScopeLock lock(m_QueueMutex);
	if (m_Queue.empty())
	{
		m_Queue.push_back(pTask);
	}
	else
	{
		bool isInserted = false;
		for (auto it = m_Queue.begin(); it != m_Queue.end(); ++it)
		{
			if ((*it)->Priority < pTask->Priority)
			{
				m_Queue.insert(it, pTask);
				isInserted = true;
				break;
			}
		}
		if (isInserted == false)
		{
			m_Queue.push_back(pTask);
		}
	}
}

void AsyncTaskQueue::Stop()
{
	m_Shutdown = true;
}

void AsyncTaskQueue::ParallelFor(int count, const ParallelForDelegate& function, bool singleThreaded /* = false */)
{
	if (singleThreaded)
	{
		for (int i = 0; i < count; ++i)
		{
			function.ExecuteIfBound(i);
		}
	}
	else
	{
		for (int i = 0; i < count; ++i)
		{
			AddWorkItem(AsyncTaskDelegate::CreateLambda([function, i](AsyncTask*, int) { function.ExecuteIfBound(i); }));
		}
	}
}

bool AsyncTaskQueue::IsCompleted() const
{
	for (const AsyncTask* pItem : m_RunningTasks)
	{
		if (pItem->IsCompleted == false)
		{
			return false;
		}
	}
	return true;
}
