#pragma once

template<typename RetVal, typename ...Args>
class IDelegate
{
public:
	virtual RetVal Execute(Args ...args) = 0;
};

template<typename RetVal, typename T, typename ...Args>
class RawDelegate : public IDelegate<RetVal, Args...>
{
public:
	using DelegateFunction = RetVal(T::*)(Args...);

	void Bind(T* pObject, DelegateFunction pFunction)
	{
		m_pObject = pObject;
		m_pFunction = pFunction;
	}

	virtual RetVal Execute(Args ...args) override
	{
		return (m_pObject->*m_pFunction)(args...);
	}

	T* m_pObject;
	DelegateFunction m_pFunction;
};

template<typename RetVal, typename...Args>
class StaticDelegate : public IDelegate<RetVal, Args...>
{
public:
	using DelegateFunction = RetVal(*)(Args...);

	void Bind(DelegateFunction pFunction)
	{
		m_pFunction = pFunction;
	}

	virtual RetVal Execute(Args ...args) override
	{
		return (*m_pFunction)(args...);
	}

	DelegateFunction m_pFunction;
};

template< typename TLambda, typename RetVal, typename... Args>
class LambdaDelegate : public IDelegate<RetVal, Args...>
{
public:
	LambdaDelegate(TLambda&& lambda) : 
		m_Lambda(new TLambda(lambda)) 
	{}
	~LambdaDelegate()
	{
		delete m_Lambda;
	}

	RetVal Execute(Args... args) override
	{
		return (*m_Lambda)(args...);
	}

private:
	TLambda* m_Lambda;
};


template<typename RetVal, typename ...Args>
class SinglecastDelegate
{
public:
	SinglecastDelegate() {}
	~SinglecastDelegate()
	{
		SafeDelete(m_pEvent);
	}

	template<typename T>
	void BindRaw(T* pObject, RetVal(T::*pFunction)(Args...))
	{
		RawDelegate<RetVal, T, Args...>* pDelegate = new RawDelegate<RetVal, T, Args...>();
		pDelegate->Bind(pObject, pFunction);
		SafeDelete(m_pEvent);
		m_pEvent = pDelegate;
	}

	void BindStatic(RetVal(*pFunction)(Args...))
	{
		StaticDelegate<RetVal, Args...>* pDelegate = new StaticDelegate<RetVal, Args...>();
		pDelegate->Bind(pFunction);
		SafeDelete(m_pEvent);
		m_pEvent = pDelegate;
	}

	template<typename LambdaType>
	void BindLamda(LambdaType&& lambda)
	{
		LambdaDelegate<LambdaType, RetVal, Args...>* pDelegate = new LambdaDelegate<LambdaType, RetVal, Args...>(std::forward<LambdaType>(lambda));
		SafeDelete(m_pEvent);
		m_pEvent = pDelegate;
	}

	RetVal ExecuteIfBound(Args ...args)
	{
		if (IsBound())
			return m_pEvent->Execute(args...);
		return RetVal();
	}

	RetVal Execute(Args ...args)
	{
		return m_pEvent->Execute(args...);
	}

	bool IsBound()
	{
		return m_pEvent != nullptr;
	}

private:
	IDelegate<RetVal, Args...>* m_pEvent = nullptr;
};

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
	DelegateHandle AddRaw(T* pObject, void(T::*pFunction)(Args...))
	{
		RawDelegate<void, T, Args...>* pDelegate = new RawDelegate<void, T, Args...>();
		pDelegate->Bind(pObject, pFunction);
		DelegateHandle handle(true);
		m_Events.push_back(EventPair(handle, pDelegate));
		return handle;
	}

	DelegateHandle AddStatic(void(*pFunction)(Args...))
	{
		StaticDelegate<void, Args...>* pDelegate = new StaticDelegate<void, Args...>();
		pDelegate->Bind(pFunction);
		DelegateHandle handle(true);
		m_Events.push_back(EventPair(handle, pDelegate));
		return handle;
	}

	template<typename LambdaType>
	DelegateHandle AddLamda(LambdaType&& lambda)
	{
		LambdaDelegate<LambdaType, void, Args...>* pDelegate = new LambdaDelegate<LambdaType, void, Args...>(std::forward<LambdaType>(lambda));
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
				SafeDelete(m_Events[i].second);
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
	using EventPair = std::pair<DelegateHandle, IDelegate<void, Args...>*>;
	std::vector<EventPair> m_Events;
};