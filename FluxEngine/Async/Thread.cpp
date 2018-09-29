#include "FluxEngine.h"
#include "Thread.h"
#include "AsyncTaskQueue.h"

#ifndef WIN32
#include <thread>
#endif

unsigned int Thread::m_MainThread;

Thread::Thread()
{
}

Thread::~Thread()
{
	StopThread();
}

bool Thread::RunThread()
{
#ifdef WIN32
	if (m_pHandle)
		return false;
	m_pHandle = CreateThread(nullptr, 0, ThreadFunctionStatic, this, 0, &m_ThreadId);
	if (m_pHandle == nullptr)
	{
		auto error = GetLastError();
		FLUX_LOG_HR("[Thread::Run()]", HRESULT_FROM_WIN32(error));
		return false;
	}
	return true;
#else
	m_pHandle = new std::thread(ThreadFunctionStatic, this);
	return true;
#endif
}

void Thread::StopThread()
{
	if (!m_pHandle)
		return;
#ifdef WIN32
	WaitForSingleObject((HANDLE)m_pHandle, INFINITE);
	if (CloseHandle((HANDLE)m_pHandle) == 0)
	{
		auto error = GetLastError();
		FLUX_LOG_HR("[Thread::Stop()]", HRESULT_FROM_WIN32(error));
	}
	m_pHandle = nullptr;
#else
	((std::thread*)m_pHandle)->join();
	delete (std::thread*)m_pHandle;
	m_pHandle = nullptr;
#endif
}

bool Thread::SetPriority(const int priority)
{
#ifdef WIN32
	if (m_pHandle)
	{
		if (SetThreadPriority((HANDLE)m_pHandle, priority) == 0)
		{
			auto error = GetLastError();
			FLUX_LOG_HR("[Thread::SetPriority()]", HRESULT_FROM_WIN32(error));
			return false;
		}
		return true;
	}
	return false;
#else
	priority;
	return false;
#endif
}

unsigned int Thread::GetCurrentId()
{
#ifdef WIN32
	return ::GetCurrentThreadId();
#else
	return (unsigned int)std::hash<std::thread::id>{}(std::this_thread::get_id());
#endif
}

DWORD WINAPI Thread::ThreadFunctionStatic(void* pData)
{
	Thread* pThread = static_cast<Thread*>(pData);
	return pThread->ThreadFunction();
}


void Thread::SetMainThread()
{
	m_MainThread = GetCurrentId();
}

bool Thread::IsMainThread()
{
	return m_MainThread == GetCurrentId();
}

bool Thread::IsMainThread(unsigned int id)
{
	return m_MainThread == id;
}

//WORKER THREAD

WorkerThread::WorkerThread(AsyncTaskQueue* pOwner, int index) :
	m_pOwner(pOwner), m_Index(index)
{
}

int WorkerThread::ThreadFunction()
{
	m_pOwner->ProcessItems(m_Index);
	return 0;
}

bool WorkerThread::Run()
{
	return RunThread();
}

int WorkerThread::GetIndex() const
{
	return m_Index;
}
