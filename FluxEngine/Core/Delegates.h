#pragma once

class DelegateHandle
{
public:
	DelegateHandle() : m_Id(0) {}
	DelegateHandle(bool generateId) : m_Id(GetNewID()) { UNREFERENCED_PARAMETER(generateId); }
	~DelegateHandle() {}

	bool operator==(const DelegateHandle& other)
	{
		return m_Id == other.m_Id;
	}

private:
	int m_Id;
	static int CURRENT_ID;
	static int GetNewID();
};

template<typename ...Args>
class ICallable
{
public:
	virtual void Execute(Args ...args) = 0;
};

template<typename T, typename ...Args>
class BaseDelegate : public ICallable<Args...>
{
public:
	using DelegateFunction = void(T::*)(Args...);

	void Bind(T* pObject, DelegateFunction pFunction)
	{
		m_pObject = pObject;
		m_pFunction = pFunction;
	}

	T* m_pObject;
	DelegateFunction m_pFunction;

	virtual void Execute(Args ...args) override
	{
		(m_pObject->*m_pFunction)(args...);
	}
};

template<typename ...Args>
class MulticastDelegate
{
public:
	MulticastDelegate() {}
	~MulticastDelegate() 
	{
		for (EventPair& e : m_Events)
			delete e.second;
		m_Events.clear();
	}

	template<typename T>
	DelegateHandle Add(T* pObject, void(T::*pFunction)(Args...))
	{
		BaseDelegate<T, Args...>* pDelegate = new BaseDelegate<T, Args...>();
		pDelegate->Bind(pObject, pFunction);
		DelegateHandle handle(true);
		m_Events.push_back(EventPair(handle, pDelegate));
		return handle;
	}

	template<typename T>
	void Add(T&& lambda)
	{
		void(*name)(void) = lambda;
	}

	bool Remove(const DelegateHandle& handle)
	{
		for (size_t i = 0; i < m_Events.size(); ++i)
		{
			if (m_Events[i].first == handle)
			{
				m_Events.erase(m_Events.begin() + i);
				return true;
			}
		}
		return false;
	}

	void Broadcast(Args ...args)
	{
		for (EventPair& e : m_Events)
			e.second->Execute(args...);
	}

private:
	using EventPair = std::pair<DelegateHandle, ICallable<Args...>*>;
	std::vector<EventPair> m_Events;
};

template<typename ...Args>
class Delegate
{
public:
	Delegate() {}
	~Delegate() 
	{
		if (m_pEvent)
		{
			delete m_pEvent;
			m_pEvent = nullptr;
		}
	}

	template<typename T>
	void Bind(T* pObject, void(T::*pFunction)(Args...))
	{
		BaseDelegate<T, Args...>* pDelegate = new BaseDelegate<T, Args...>();
		pDelegate->Bind(pObject, pFunction);
		m_pEvent = pDelegate;
	}

	void ExecuteIfBound(Args ...args)
	{
		if (m_pEvent)
			m_pEvent->Execute(args...);
	}

	void Execute(Args ...args)
	{
		m_pEvent->Execute(args...);
	}

private:
	ICallable<Args...>* m_pEvent;
};