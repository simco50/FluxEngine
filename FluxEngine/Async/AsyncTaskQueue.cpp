#include "FluxEngine.h"
#include "AsyncTaskQueue.h"
#include "Thread.h"

AsyncTaskQueue::AsyncTaskQueue(const size_t count)
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

	for (AsyncTask*& pItem : m_Items)
	{
		delete pItem;
		pItem = nullptr;
	}

	for (WorkerThread*& pThread : m_pThreads)
	{
		delete pThread;
		pThread = nullptr;
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

void AsyncTaskQueue::AddWorkItem(AsyncTask* pItem)
{
	if (pItem == nullptr)
		return;

	m_Items.push_back(pItem);

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
	for (AsyncTask* pItem : m_Items)
	{
		if (pItem->IsCompleted == false)
			return false;
	}
	return true;
}
