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

template<typename RetVal, typename ...Args>
class ICallable
{
public:
	virtual RetVal Execute(Args ...args) = 0;
};

template<typename RetVal, typename T, typename ...Args>
class BaseDelegate : public ICallable<RetVal, Args...>
{
public:
	using DelegateFunction = RetVal(T::*)(Args...);

	void Bind(T* pObject, DelegateFunction pFunction)
	{
		m_pObject = pObject;
		m_pFunction = pFunction;
	}

	T* m_pObject;
	DelegateFunction m_pFunction;

	virtual RetVal Execute(Args ...args) override
	{
		return (m_pObject->*m_pFunction)(args...);
	}
};

template<typename RetVal, typename ...Args>
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

	template<typename RetVal, typename T>
	void Bind(T* pObject, RetVal(T::*pFunction)(Args...))
	{
		BaseDelegate<RetVal, T, Args...>* pDelegate = new BaseDelegate<RetVal, T, Args...>();
		pDelegate->Bind(pObject, pFunction);
		m_pEvent = pDelegate;
	}

	RetVal ExecuteIfBound(Args ...args)
	{
		if (m_pEvent)
			return m_pEvent->Execute(args...);
		return (RetVal)0;
	}

	RetVal Execute(Args ...args)
	{
		return m_pEvent->Execute(args...);
	}

private:
	ICallable<RetVal, Args...>* m_pEvent;
};

template<typename ...Args >
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
		BaseDelegate<void, T, Args...>* pDelegate = new BaseDelegate<void, T, Args...>();
		pDelegate->Bind(pObject, pFunction);
		DelegateHandle handle(true);
		m_Events.push_back(EventPair(handle, pDelegate));
		return handle;
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
	using EventPair = std::pair<DelegateHandle, ICallable<void, Args...>*>;
	std::vector<EventPair> m_Events;
};