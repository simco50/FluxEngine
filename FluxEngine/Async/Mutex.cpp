#include "FluxEngine.h"
#include "Mutex.h"

Mutex::Mutex() :
	m_pHandle(new CRITICAL_SECTION())
{
	InitializeCriticalSection((CRITICAL_SECTION*)m_pHandle);
}

Mutex::~Mutex()
{
	CRITICAL_SECTION* pCs = (CRITICAL_SECTION*)m_pHandle;
	DeleteCriticalSection(pCs);
	delete pCs;
	pCs = nullptr;
}

void Mutex::Lock()
{
	EnterCriticalSection((CRITICAL_SECTION*)m_pHandle);
}

bool Mutex::TryLock()
{
	return TryEnterCriticalSection((CRITICAL_SECTION*)m_pHandle) != FALSE;
}

void Mutex::Unlock()
{
	LeaveCriticalSection((CRITICAL_SECTION*)m_pHandle);
}
