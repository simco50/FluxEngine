#pragma once

#include <assert.h>
#include <memory>
#include <map>

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

	explicit StaticDelegate(DelegateFunction pFunction) :
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
template<typename TLambda, typename RetVal, typename... Args>
class LambdaDelegate : public IDelegate<RetVal, Args...>
{
public:
	explicit LambdaDelegate(TLambda&& lambda) : 
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

	SPDelegate(const std::shared_ptr<T>& pObject, DelegateFunction pFunction) :
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

template<typename RetVal, typename ...Args>
class DelegateWrapper
{
public:
	template<typename T>
	DelegateWrapper(T* pObj, RetVal(T::*pFunction)(Args...)) :
		m_pDelegate(new RawDelegate<RetVal, T, Args...>(pObj, pFunction))
	{}

	DelegateWrapper(RetVal(*pFunction)(Args...)) :
		m_pDelegate(new StaticDelegate<RetVal, Args...>(pFunction))
	{}

	template<typename T>
	DelegateWrapper(const std::shared_ptr<T>& pObj, RetVal(T::*pFunction)(Args...)) :
		m_pDelegate(new SPDelegate<RetVal, T, Args...>(pObj, pFunction))
	{}

	template<typename LambdaType>
	DelegateWrapper(LambdaType&& lambda) :
		m_pDelegate(new LambdaDelegate<LambdaType, RetVal, Args...>(std::forward<LambdaType>(lambda)))
	{}

	~DelegateWrapper()
	{
		if (m_pDelegate)
		{
			delete m_pDelegate;
			m_pDelegate = nullptr;
		}
	}

	RetVal Execute(Args... args)
	{
		if (m_pDelegate)
			return m_pDelegate->Execute(args...);
		return RetVal();
	}

private:
	IDelegate<RetVal, Args...>* m_pDelegate = nullptr;
};

//Delegate that can be bound to by just ONE object
template<typename RetVal, typename ...Args>
class SinglecastDelegate
{
public:
	SinglecastDelegate() {}
	~SinglecastDelegate()
	{
		if (m_pEvent)
		{
			delete m_pEvent;
			m_pEvent = nullptr;
		}
	}

	SinglecastDelegate(const SinglecastDelegate& other) = delete;
	SinglecastDelegate& operator=(const SinglecastDelegate& other) = delete;

	//Create a SinglecastDelegate instance bound with member function
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
		Release();
		m_pEvent = new DelegateWrapper<RetVal, Args...>(pObject, pFunction);
	}

	//Create a SinglecastDelegate instance bound with a static/global function
	static SinglecastDelegate CreateStatic(RetVal(*pFunction)(Args...))
	{
		SinglecastDelegate<RetVal, Args...> NewDelegate;
		NewDelegate.BindStatic(pFunction);
		return NewDelegate;
	}

	//Bind a static/global function
	void BindStatic(RetVal(*pFunction)(Args...))
	{
		Release();
		m_pEvent = new DelegateWrapper<RetVal, Args...>(pFunction);
	}

	//Create a SinglecastDelegate instance bound with a lambda
	template<typename LambdaType>
	static SinglecastDelegate CreateLambda(LambdaType&& lambda)
	{
		SinglecastDelegate<RetVal, Args...> NewDelegate;
		NewDelegate.BindLambda(lambda);
		return NewDelegate;
	}

	//Bind a lambda
	template<typename LambdaType>
	void BindLambda(LambdaType&& lambda)
	{
		Release();
		m_pEvent = new DelegateWrapper<RetVal, Args...>(std::forward<LambdaType>(lambda));
	}

	//Create a SinglecastDelegate instance bound with member function using a shared_ptr
	template<typename T>
	static SinglecastDelegate CreateSP(std::shared_ptr<T> pObject, RetVal(T::*pFunction)(Args...))
	{
		SinglecastDelegate<RetVal, Args...> NewDelegate;
		NewDelegate.BindStatic(pObject, pFunction);
		return NewDelegate;
	}

	//Bind a member function with a shared_ptr object
	template<typename T>
	void BindSP(std::shared_ptr<T> pObject, RetVal(T::*pFunction)(Args...))
	{
		Release();
		m_pEvent = new DelegateWrapper<RetVal, Args...>(pObject, pFunction);
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
		assert(m_pEvent != nullptr);
		return m_pEvent->Execute(args...);
	}

	//Check if there is a function bound
	bool IsBound() const
	{
		return m_pEvent != nullptr;
	}

	void Clear()
	{
		Release();
	}

private:
	void Release()
	{
		if (m_pEvent)
		{
			delete m_pEvent;
			m_pEvent = nullptr;
		}
	}

	DelegateWrapper<RetVal, Args...>* m_pEvent = nullptr;
};

//A handle to a delegate used for a multicast delegate
//Static ID so that every handle is unique
class DelegateHandle
{
public:
	DelegateHandle() : m_Id(-1) {}
	explicit DelegateHandle(bool generateId) : m_Id(GetNewID()) { UNREFERENCED_PARAMETER(generateId); }
	~DelegateHandle() {}

	bool operator==(const DelegateHandle& other) const
	{
		return m_Id == other.m_Id;
	}
	bool operator<(const DelegateHandle& other) const
	{
		return m_Id < other.m_Id;
	}

	bool IsValid() const { return m_Id != -1; }
	void Invalidate() { m_Id = -1; }

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
		for (auto& e : m_Events)
			delete e.second;
		m_Events.clear();
	}

	MulticastDelegate(const MulticastDelegate& other) = delete;
	MulticastDelegate& operator=(const MulticastDelegate& other) = delete;

	DelegateHandle operator+=(DelegateWrapper<void, Args...>* pDelegate)
	{
		return AddDelegate(pDelegate);
	}

	bool operator-=(DelegateHandle& handle)
	{
		return Remove(handle);
	}

	//Bind a member function
	template<typename T>
	DelegateHandle AddRaw(T* pObject, void(T::*pFunction)(Args...))
	{
		DelegateWrapper<void, Args...>* pDelegate = new DelegateWrapper<void, Args...>(pObject, pFunction);
		return AddDelegate(pDelegate);
	}

	//Bind a static/global function
	DelegateHandle AddStatic(void(*pFunction)(Args...))
	{
		DelegateWrapper<void, Args...>* pDelegate = new DelegateWrapper<void, Args...>(pFunction);
		return AddDelegate(pDelegate);
	}

	//Bind a lambda
	template<typename LambdaType>
	DelegateHandle AddLambda(LambdaType&& lambda)
	{
		DelegateWrapper<void, Args...>* pDelegate = new DelegateWrapper<void, Args...>(std::forward<LambdaType>(lambda));
		return AddDelegate(pDelegate);
	}

	//Bind a member function with a shared_ptr object
	template<typename T>
	DelegateHandle AddSP(std::shared_ptr<T> pObject, void(T::*pFunction)(Args...))
	{
		DelegateWrapper<void, Args...>* pDelegate = new DelegateWrapper<void, Args...>(pObject, pFunction);
		return AddDelegate(pDelegate);
	}

	//Remove a function from the event list by the handle
	bool Remove(DelegateHandle& handle)
	{
		auto pIt = m_Events.find(handle);
		if (pIt != m_Events.end())
		{
			if (pIt->second)
				delete pIt->second;
			m_Events.erase(pIt);
			return true;
		}
		return false;
	}

	//Remove all the functions bound to the delegate
	void RemoveAll()
	{
		for (auto& delegatePair : m_Events)
		{
			if (delegatePair.second)
				delete delegatePair.second;
		}
		m_Events.clear();
	}

	//Execute all functions that are bound
	void Broadcast(Args ...args)
	{
		for (auto& e : m_Events)
			e.second->Execute(args...);
	}

private:
	DelegateHandle AddDelegate(DelegateWrapper<void, Args...>* pDelegate)
	{
		DelegateHandle handle(true);
		m_Events[handle] = pDelegate;
		return handle;
	}

	std::map<DelegateHandle, DelegateWrapper<void, Args...>*> m_Events;
};