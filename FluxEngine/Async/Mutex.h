#pragma once

class Mutex
{
public:
	Mutex();
	~Mutex();

	void Lock();
	bool TryLock();
	void Unlock();

private:
	void* m_pHandle = nullptr;
};

class ScopeLock
{
public:
	ScopeLock(Mutex& mutex) :
		m_pMutex(&mutex)
	{
		m_pMutex->Lock();
	}
	~ScopeLock()
	{
		m_pMutex->Unlock();
	}

private:
	Mutex* m_pMutex;
};