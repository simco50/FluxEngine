#include "FluxEngine.h"
#include "Thread.h"

#define USE_STD 0

#if USE_STD
#include <thread>
#endif

Thread::Thread()
{
}

Thread::~Thread()
{
	Stop();
}

bool Thread::Run()
{
#if USE_STD
	m_pHandle = new std::thread(ThreadFunctionStatic, this);
#else
	if (m_pHandle)
		return false;
	m_pHandle = CreateThread(nullptr, 0, ThreadFunctionStatic, this, 0, &m_ThreadId);
	if (m_pHandle == nullptr)
	{
		auto error = GetLastError();
		FLUX_LOG_HR("[Thread::Run()]", HRESULT_FROM_WIN32(error));
		return false;
	}
#endif
	return true;
}

void Thread::Stop()
{
	if (!m_pHandle)
		return;
#if USE_STD
	((std::thread*)m_pHandle)->join();
	delete (std::thread*)m_pHandle;
	m_pHandle = nullptr;
#else
	WaitForSingleObject((HANDLE)m_pHandle, INFINITE);
	if (CloseHandle((HANDLE)m_pHandle) == 0)
	{
		auto error = GetLastError();
		FLUX_LOG_HR("[Thread::Stop()]", HRESULT_FROM_WIN32(error));
	}
	m_pHandle = nullptr;
#endif
}

bool Thread::SetPriority(const int priority)
{
#if USE_STD
	UNREFERENCED_PARAMETER(priority);
	return false;
#else
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
#endif
}

unsigned int Thread::GetCurrentId()
{
#if USE_STD
	return (unsigned int)std::hash<std::thread::id>{}(std::this_thread::get_id());
#else
	return ::GetCurrentThreadId();
#endif
}

DWORD WINAPI Thread::ThreadFunctionStatic(void* pData)
{
	Thread* pThread = static_cast<Thread*>(pData);
	return pThread->ThreadFunction();
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
