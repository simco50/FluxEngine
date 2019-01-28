#include "FluxEngine.h"
#include "Thread.h"
#include "AsyncTaskQueue.h"

#ifndef WIN32
#include <thread>
#endif

unsigned int Thread::m_MainThread = 0;

Thread::Thread()
{
}

Thread::~Thread()
{
	StopThread();
}

bool Thread::RunThread(ThreadFunction function, void* pArgs)
{
#ifdef WIN32
	if (m_pHandle)
	{
		return false;
	}
	m_pHandle = CreateThread(nullptr, 0, function, pArgs, 0, (DWORD*)&m_ThreadId);
	if (m_pHandle == nullptr)
	{
		auto error = GetLastError();
		FLUX_LOG_HR("[Thread::Run()]", HRESULT_FROM_WIN32(error));
		return false;
	}
	return true;
#else
	m_pHandle = new std::thread(ThreadFunctionStatic, pArgs);
	return true;
#endif
}

void Thread::StopThread()
{
	if (!m_pHandle)
	{
		return;
	}
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

void Thread::SetAffinity(const uint64 affinity)
{
	SetAffinity(m_pHandle, affinity);
}

void Thread::SetAffinity(void* pHandle, const uint64 affinity)
{
	check(pHandle);
	::SetThreadAffinityMask((HANDLE*)pHandle, (DWORD)affinity);
}

void Thread::LockToCore(const uint32 core)
{
	uint32 affinity = 1 << core;
	SetAffinity(m_pHandle, (uint64)affinity);
}

void Thread::SetCurrentAffinity(const uint64 affinity)
{
	SetAffinity(GetCurrentThread(), affinity);
}

void Thread::LockCurrentToCore(const uint32 core)
{
	uint32 affinity = 1 << core;
	SetAffinity(GetCurrentThread(), (uint64)affinity);
}

uint32 Thread::GetCurrentId()
{
#ifdef WIN32
	return (uint32)::GetCurrentThreadId();
#else
	return (uint32)std::hash<std::thread::id>{}(std::this_thread::get_id());
#endif
}

void Thread::SetMainThread()
{
	m_MainThread = GetCurrentId();
}

bool Thread::IsMainThread()
{
	return m_MainThread == GetCurrentId();
}

bool Thread::IsMainThread(uint32 id)
{
	return m_MainThread == id;
}

void Thread::SetName(uint32 id, const std::string& name)
{
#if _MSC_VER
	const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} THREADNAME_INFO;
#pragma pack(pop)

		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = name.c_str();
		info.dwThreadID = id;
		info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
		__try {
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
		}
#pragma warning(pop)
#else
	UNREFERENCED_PARAMETER(id);
	UNREFERENCED_PARAMETER(name);
#endif
}

bool HookableThread::RunThread()
{
	return Thread::RunThread(&HookableThread::ThreadFunctionStatic, this);
}

//HOOKABLE THREAD

DWORD WINAPI HookableThread::ThreadFunctionStatic(void* pData)
{
	HookableThread* pThread = static_cast<HookableThread*>(pData);
	return (DWORD)pThread->ThreadFunction();
}

//WORKER THREAD

WorkerThread::WorkerThread(AsyncTaskQueue* pOwner, int index)
	: m_pOwner(pOwner), m_Index(index)
{
}

int WorkerThread::ThreadFunction()
{
	m_pOwner->ProcessItems(m_Index);
	return 0;
}

int WorkerThread::GetIndex() const
{
	return m_Index;
}
