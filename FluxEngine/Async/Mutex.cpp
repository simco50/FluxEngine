#include "FluxEngine.h"
#include "Mutex.h"

#ifndef WIN32
#include <mutex>
#endif

Mutex::Mutex()
{
#ifdef WIN32
	m_pHandle = new CRITICAL_SECTION();
	InitializeCriticalSection((CRITICAL_SECTION*)m_pHandle);
#else
	m_pHandle = new std::mutex();
#endif
}

Mutex::~Mutex()
{
#ifdef WIN32
	CRITICAL_SECTION* pCs = (CRITICAL_SECTION*)m_pHandle;
	DeleteCriticalSection(pCs);
	delete pCs;
	pCs = nullptr;
#else
	delete (std::mutex*)m_pHandle;
#endif
}

void Mutex::Lock()
{
#ifdef WIN32
	EnterCriticalSection((CRITICAL_SECTION*)m_pHandle);
#else
	((std::mutex*)m_pHandle)->lock();
#endif
}

bool Mutex::TryLock()
{
#ifdef WIN32
	return TryEnterCriticalSection((CRITICAL_SECTION*)m_pHandle) != FALSE;
#else
	return ((std::mutex*)m_pHandle)->try_lock();
#endif
}

void Mutex::Unlock()
{
#ifdef WIN32
	LeaveCriticalSection((CRITICAL_SECTION*)m_pHandle);
#else
	((std::mutex*)m_pHandle)->unlock();
#endif
}
