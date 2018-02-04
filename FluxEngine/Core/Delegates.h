#pragma once

//Base type for delegates
template<typename RetVal, typename ...Args>
class IDelegate
{
public:
	virtual ~IDelegate() {}
	virtual RetVal Execute(Args ...args) = 0;
};

//Delegate for member functions
template<typename RetVal, typename T, typename ...Args>
class RawDelegate : public IDelegate<RetVal, Args...>
{
public:
	using DelegateFunction = RetVal(T::*)(Args...);

	RawDelegate(T* pObject, DelegateFunction pFunction) :
		m_pObject(pObject),
		m_pFunction(pFunction)
	{}

	virtual RetVal Execute(Args ...args) override
	{
		return (m_pObject->*m_pFunction)(args...);
	}

private:
	T* m_pObject;
	DelegateFunction m_pFunction;
};

//Delegate for global/static functions
template<typename RetVal, typename...Args>
class StaticDelegate : public IDelegate<RetVal, Args...>
{
public:
	using DelegateFunction = RetVal(*)(Args...);

	StaticDelegate(DelegateFunction pFunction) :
		m_pFunction(pFunction)
	{}

	virtual RetVal Execute(Args ...args) override
	{
		return (*m_pFunction)(args...);
	}

private:
	DelegateFunction m_pFunction;
};

//Delegate for lambdas
template< typename TLambda, typename RetVal, typename... Args>
class LambdaDelegate : public IDelegate<RetVal, Args...>
{
public:
	LambdaDelegate(TLambda&& lambda) : 
		m_Lambda(new TLambda(lambda)) 
	{}
	virtual ~LambdaDelegate()
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

//Delegate for shared pointers
template<typename RetVal, typename T, typename ...Args>
class SPDelegate : public IDelegate<RetVal, Args...>
{
public:
	using DelegateFunction = RetVal(T::*)(Args...);

	SPDelegate(shared_ptr<T> pObject, DelegateFunction pFunction) :
		m_pObject(pObject),
		m_pFunction(pFunction)
	{}
	virtual RetVal Execute(Args ...args) override
	{
		return (m_pObject.get()->*m_pFunction)(args...);
	}

private:
	std::shared_ptr<T> m_pObject;
	DelegateFunction m_pFunction;
};

//Delegate that can be bound to by just ONE object
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
	static SinglecastDelegate CreateRaw(T* pObject, RetVal(T::*pFunction)(Args...))
	{
		SinglecastDelegate<RetVal, Args...> NewDelegate;
		NewDelegate.BindRaw(pObject, pFunction);
		return NewDelegate;
	}

	//Bind a member function
	template<typename T>
	void BindRaw(T* pObject, RetVal(T::*pFunction)(Args...))
	{
		RawDelegate<RetVal, T, Args...>* pDelegate = new RawDelegate<RetVal, T, Args...>(pObject, pFunction);
		SafeDelete(m_pEvent);
		m_pEvent = pDelegate;
	}

	//Bind a static/global function
	void BindStatic(RetVal(*pFunction)(Args...))
	{
		StaticDelegate<RetVal, Args...>* pDelegate = new StaticDelegate<RetVal, Args...>(pFunction);
		SafeDelete(m_pEvent);
		m_pEvent = pDelegate;
	}

	//Bind a lambda
	template<typename LambdaType>
	void BindLambda(LambdaType&& lambda)
	{
		LambdaDelegate<LambdaType, RetVal, Args...>* pDelegate = new LambdaDelegate<LambdaType, RetVal, Args...>(std::forward<LambdaType>(lambda));
		SafeDelete(m_pEvent);
		m_pEvent = pDelegate;
	}

	//Bind a member function with a shared_ptr object
	template<typename T>
	void BindSP(shared_ptr<T> pObject, RetVal(T::*pFunction)(Args...))
	{
		SPDelegate<RetVal, T, Args...>* pDelegate = new SPDelegate<RetVal, T, Args...>(pObject, pFunction);
		SafeDelete(m_pEvent);
		m_pEvent = pDelegate;
	}

	//Execute the function if the delegate is bound
	RetVal ExecuteIfBound(Args ...args)
	{
		if (IsBound())
			return m_pEvent->Execute(args...);
		return RetVal();
	}

	//Execute the function
	RetVal Execute(Args ...args)
	{
		checkf(IsBound(), "[SinglecastDelegate::Execute] > Delegate is not bound");
		return m_pEvent->Execute(args...);
	}

	//Check if there is a function bound
	bool IsBound()
	{
		return m_pEvent != nullptr;
	}

private:
	IDelegate<RetVal, Args...>* m_pEvent = nullptr;
};

//A handle to a delegate used for a multicast delegate
//Static ID so that every handle is unique
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

//Delegate that can be bound to by MULTIPLE objects
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

	//Bind a member function
	template<typename T>
	DelegateHandle AddRaw(T* pObject, void(T::*pFunction)(Args...))
	{
		RawDelegate<void, T, Args...>* pDelegate = new RawDelegate<void, T, Args...>(pObject, pFunction);
		DelegateHandle handle(true);
		m_Events.push_back(EventPair(handle, pDelegate));
		return handle;
	}

	//Bind a static/global function
	DelegateHandle AddStatic(void(*pFunction)(Args...))
	{
		StaticDelegate<void, Args...>* pDelegate = new StaticDelegate<void, Args...>(pFunction);
		DelegateHandle handle(true);
		m_Events.push_back(EventPair(handle, pDelegate));
		return handle;
	}

	//Bind a lambda
	template<typename LambdaType>
	DelegateHandle AddLambda(LambdaType&& lambda)
	{
		LambdaDelegate<LambdaType, void, Args...>* pDelegate = new LambdaDelegate<LambdaType, void, Args...>(std::forward<LambdaType>(lambda));
		DelegateHandle handle(true);
		m_Events.push_back(EventPair(handle, pDelegate));
		return handle;
	}

	//Bind a member function with a shared_ptr object
	template<typename T>
	void AddSP(shared_ptr<T> pObject, void(T::*pFunction)(Args...))
	{
		SPDelegate<void, T, Args...>* pDelegate = new SPDelegate<void, T, Args...>(pObject, pFunction);
		DelegateHandle handle(true);
		SafeDelete(m_pEvent);
		m_Events.push_back(EventPair(handle, pDelegate));
		return handle;
	}

	//Remove a function from the event list by the handle
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

	//Remove all the functions bound to the delegate
	void RemoveAll()
	{
		for (EventPair delegatePair : m_Events)
		{
			SafeDelete(delegatePair.second);
		}
		m_Events.clear();
	}

	//Execute all functions that are bound
	void Broadcast(Args ...args)
	{
		for (EventPair& e : m_Events)
			e.second->Execute(args...);
	}

private:
	using EventPair = std::pair<DelegateHandle, IDelegate<void, Args...>*>;
	std::vector<EventPair> m_Events;
};