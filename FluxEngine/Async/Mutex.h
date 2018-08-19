#pragma once

class Mutex
{
public:
	Mutex();
	~Mutex();

	DELETE_COPY(Mutex)

	void Lock();
	bool TryLock();
	void Unlock();

private:
	void* m_pHandle = nullptr;
};

class ScopeLock
{
public:
	explicit ScopeLock(Mutex& mutex) :
		m_pMutex(&mutex)
	{
		m_pMutex->Lock();
	}
	~ScopeLock()
	{
		m_pMutex->Unlock();
	}

	DELETE_COPY(ScopeLock)

private:
	Mutex* m_pMutex;
};