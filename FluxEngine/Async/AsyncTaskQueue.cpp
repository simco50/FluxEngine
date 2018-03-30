#include "FluxEngine.h"
#include "AsyncTaskQueue.h"
#include "Thread.h"

AsyncTaskQueue::AsyncTaskQueue(Context* pContext, const size_t count):
	Subsystem(pContext)
{
#ifdef THREADING
	CreateThreads(count);
#else
	UNREFERENCED_PARAMETER(count);
#endif
}

AsyncTaskQueue::~AsyncTaskQueue()
{
	m_Shutdown = true;

	m_Tasks.clear();

	for (WorkerThread*& pThread : m_pThreads)
	{
		delete pThread;
		pThread = nullptr;
	}
	for (AsyncTask*& pTask : m_Tasks)
	{
		SafeDelete(pTask);
	}
	for (AsyncTask*& pTask : m_TaskPool)
	{
		SafeDelete(pTask);
	}
}

void AsyncTaskQueue::CreateThreads(const size_t count)
{
	if (m_pThreads.size() > 0)
		return;
	for (size_t i = 0; i < count; ++i)
	{
		WorkerThread* pThread = new WorkerThread(this, (int)i);
		pThread->Run();
		m_pThreads.push_back(pThread);
	}
}

void AsyncTaskQueue::JoinAll()
{
	m_Paused = false;
#ifdef THREADING
	for (;;)
	{
		if (IsCompleted())
			break;
	}
#else
	//Execute all of them on the main thread (current)
	for (AsyncTask* pTask : m_Queue)
		pTask->Action.ExecuteIfBound(pTask, 0);
	m_Queue.clear();
#endif

	for (auto& pTask : m_Tasks)
	{
		pTask->Action.Clear();
		pTask->IsCompleted = false;
		pTask->Priority = 0;
		m_TaskPool.push_back(std::move(pTask));
	}
	m_Tasks.clear();
}

void AsyncTaskQueue::ProcessItems(int index)
{
	bool wasActive = false;

	for (;;)
	{
		if (m_Shutdown)
			return;

		if (m_Paused && !wasActive)
			Sleep(0);

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
	//#todo: Fix pooling. Doesn't work!
	/*if (m_TaskPool.size() > 0)
	{
		AsyncTask* pTask = m_TaskPool.back();
		m_TaskPool.pop_back();
		return pTask;
	}
	else
	{*/
		AsyncTask* pTask = new AsyncTask();
		m_TaskPool.push_back(pTask);
		return m_TaskPool[m_TaskPool.size() - 1];
	//}
}

void AsyncTaskQueue::AddWorkItem(AsyncTask* pItem)
{
	if (pItem == nullptr)
		return;

	auto pIt = std::find_if(m_TaskPool.begin(), m_TaskPool.end(), [pItem](AsyncTask* pOther) {return pOther == pItem; });
	if (pIt == m_TaskPool.end())
	{
		FLUX_LOG(Warning, "[AsyncTaskQueue::AddWorkItem] > Task is not in the pool");
		return;
	}
	m_Tasks.push_back(std::move(*pIt));
	m_TaskPool.erase(pIt);

	m_QueueMutex.Lock();
	if (m_Queue.empty())
	{
		m_Queue.push_back(pItem);
	}
	else
	{
		bool isInserted = false;
		for (auto it = m_Queue.begin(); it != m_Queue.end(); ++it)
		{
			if ((*it)->Priority < pItem->Priority)
			{
				m_Queue.insert(it, pItem);
				isInserted = true;
				break;
			}
		}
		if(isInserted == false)
			m_Queue.push_back(pItem);
	}
	m_QueueMutex.Unlock();
}

void AsyncTaskQueue::Stop()
{
	m_Shutdown = true;
}

bool AsyncTaskQueue::IsCompleted() const
{
	for (auto& pItem : m_Tasks)
	{
		if (pItem->IsCompleted == false)
			return false;
	}
	return true;
}
