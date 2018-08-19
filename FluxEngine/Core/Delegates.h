#pragma once

#include <assert.h>
#include <memory>

#define DECLARE_DELEGATE(name, ...) \
using name = SinglecastDelegate<void, __VA_ARGS__>

#define DECLARE_DELEGATE_RET(name, retValue, ...) \
using name = SinglecastDelegate<retValue, __VA_ARGS__>

#define DECLARE_MULTICAST_DELEGATE(name, ...) \
using name = MulticastDelegate<__VA_ARGS__>

//Base type for delegates
template<typename RetVal, typename ...Args>
class IDelegate
{
public:
	virtual ~IDelegate() {}
	virtual RetVal Execute(Args ...args) = 0;
	virtual void* GetOwner() = 0;
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

	virtual void* GetOwner() override
	{
		return m_pObject;
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

	virtual void* GetOwner() override
	{
		return nullptr;
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
		m_pLambda(std::make_shared<TLambda>(lambda)) 
	{}

	RetVal Execute(Args... args) override
	{
		return (*m_pLambda)(args...);
	}

	virtual void* GetOwner() override
	{
		return nullptr;
	}

private:
	std::shared_ptr<TLambda> m_pLambda;
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

	virtual void* GetOwner() override
	{
		return m_pObject.get();
	}

private:
	std::shared_ptr<T> m_pObject;
	DelegateFunction m_pFunction;
};

template<typename RetVal, typename ...Args>
class DelegateHandler
{
public:
	template<typename T>
	DelegateHandler(T* pObj, RetVal(T::*pFunction)(Args...)) :
		m_pDelegate(std::make_unique<RawDelegate<RetVal, T, Args...>>(pObj, pFunction))
	{}

	DelegateHandler(RetVal(*pFunction)(Args...)) :
		m_pDelegate(std::make_unique<StaticDelegate<RetVal, Args...>>(pFunction))
	{}

	template<typename T>
	DelegateHandler(const std::shared_ptr<T>& pObj, RetVal(T::*pFunction)(Args...)) :
		m_pDelegate(std::make_unique<SPDelegate<RetVal, T, Args...>>(pObj, pFunction))
	{}

	template<typename LambdaType>
	DelegateHandler(LambdaType&& lambda) :
		m_pDelegate(std::make_unique<LambdaDelegate<LambdaType, RetVal, Args...>>(std::forward<LambdaType>(lambda)))
	{}

	~DelegateHandler()
	{
	}

	RetVal Execute(Args... args)
	{
		if (m_pDelegate)
		{
			return m_pDelegate->Execute(args...);
		}
		return RetVal();
	}

	void* GetOwner()
	{
		if (m_pDelegate)
		{
			return m_pDelegate->GetOwner();
		}
		return nullptr;
	}

private:
	std::unique_ptr<IDelegate<RetVal, Args...>> m_pDelegate;
};

//Delegate that can be bound to by just ONE object
template<typename RetVal, typename ...Args>
class SinglecastDelegate
{
private:
	using DelegateHandlerT = DelegateHandler<RetVal, Args...>;
	using DelegateHandlerPtrT = std::shared_ptr<DelegateHandler<RetVal, Args...>>;

public:
	SinglecastDelegate() {}
	~SinglecastDelegate() {}

	//Create a SinglecastDelegate instance bound with member function
	template<typename T>
	static SinglecastDelegate CreateRaw(T* pObject, RetVal(T::*pFunction)(Args...))
	{
		SinglecastDelegate NewDelegate;
		NewDelegate.BindRaw(pObject, pFunction);
		return NewDelegate;
	}

	//Create a SinglecastDelegate instance bound with a static/global function
	static SinglecastDelegate CreateStatic(RetVal(*pFunction)(Args...))
	{
		SinglecastDelegate NewDelegate;
		NewDelegate.BindStatic(pFunction);
		return NewDelegate;
	}

	//Create a SinglecastDelegate instance bound with a lambda
	template<typename LambdaType>
	static SinglecastDelegate CreateLambda(LambdaType&& lambda)
	{
		SinglecastDelegate NewDelegate;
		NewDelegate.BindLambda(std::forward<LambdaType>(lambda));
		return NewDelegate;
	}

	//Create a SinglecastDelegate instance bound with member function using a shared_ptr
	template<typename T>
	static SinglecastDelegate CreateSP(std::shared_ptr<T> pObject, RetVal(T::*pFunction)(Args...))
	{
		SinglecastDelegate NewDelegate;
		NewDelegate.BindStatic(pObject, pFunction);
		return NewDelegate;
	}

	//Bind a member function
	template<typename T>
	void BindRaw(T* pObject, RetVal(T::*pFunction)(Args...))
	{
		m_pEvent = std::make_shared<DelegateHandlerT>(pObject, pFunction);
	}

	//Bind a static/global function
	void BindStatic(RetVal(*pFunction)(Args...))
	{
		m_pEvent = std::make_shared<DelegateHandlerT>(pFunction);
	}

	//Bind a lambda
	template<typename LambdaType>
	void BindLambda(LambdaType&& lambda)
	{
		m_pEvent = std::make_shared<DelegateHandlerT>(std::forward<LambdaType>(lambda));
	}

	//Bind a member function with a shared_ptr object
	template<typename T>
	void BindSP(std::shared_ptr<T> pObject, RetVal(T::*pFunction)(Args...))
	{
		m_pEvent = std::make_shared<DelegateHandlerT>(pObject, pFunction);
	}

	//Execute the function if the delegate is bound
	RetVal ExecuteIfBound(Args ...args) const
	{
		if (IsBound())
		{
			return m_pEvent->Execute(std::forward<Args>(args)...);
		}
		return RetVal();
	}

	//Execute the function
	RetVal Execute(Args ...args) const
	{
		assert(m_pEvent != nullptr);
		return m_pEvent->Execute(args...);
	}

	//Check if there is a function bound
	inline bool IsBound() const
	{
		return m_pEvent != nullptr;
	}

	inline bool IsBoundTo(void* pObject) const
	{
		return IsBound() && m_pEvent->GetOwner() == pObject;
	}

	inline void Clear()
	{
		Release();
	}

	inline void ClearIfBoundTo(void* pObject)
	{
		if (IsBoundTo(pObject))
		{
			Release();
		}
	}

private:
	inline void Release()
	{
		m_pEvent.reset();
	}

	DelegateHandlerPtrT m_pEvent;
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
template<typename ...Args>
class MulticastDelegate
{
private:
	using DelegateHandlerT = DelegateHandler<void, Args...>;
	using DelegateHandlerPtrT = std::unique_ptr<DelegateHandler<void, Args...>>;

public:
	MulticastDelegate() {}
	~MulticastDelegate() 
	{
		m_Events.clear();
	}

	MulticastDelegate(const MulticastDelegate& other) = delete;
	MulticastDelegate& operator=(const MulticastDelegate& other) = delete;

	bool operator-=(DelegateHandle& handle)
	{
		return Remove(handle);
	}

	//Bind a member function
	template<typename T>
	DelegateHandle AddRaw(T* pObject, void(T::*pFunction)(Args...))
	{
		return AddDelegate(std::make_unique<DelegateHandlerT>(pObject, pFunction));
	}

	//Bind a static/global function
	DelegateHandle AddStatic(void(*pFunction)(Args...))
	{
		return AddDelegate(std::make_unique<DelegateHandlerT>(pFunction));
	}

	//Bind a lambda
	template<typename LambdaType>
	DelegateHandle AddLambda(LambdaType&& lambda)
	{
		return AddDelegate(std::make_unique<DelegateHandlerT>(std::forward<LambdaType>(lambda)));
	}

	//Bind a member function with a shared_ptr object
	template<typename T>
	DelegateHandle AddSP(std::shared_ptr<T> pObject, void(T::*pFunction)(Args...))
	{
		return AddDelegate(pDelegatestd::make_unique<DelegateHandlerT>(pObject, pFunction));
	}

	//Removes all handles that are bound from a specific object
	//Note: Only works on Raw and SP bindings
	void RemoveObject(void* pObject)
	{
		for (size_t i = 0; i < m_Events.size(); ++i)
		{
			if (e.second->GetOwner() == pObject)
			{
				e.second.reset();
				std::swap(e, m_Events[m_Bindings - 1]);
				--m_Bindings;
			}
		}
	}

	//Remove a function from the event list by the handle
	bool Remove(DelegateHandle& handle)
	{
		for (auto& e : m_Events)
		{
			if (e.first == handle)
			{
				e.second.reset();
				std::swap(e, m_Events[m_Bindings - 1]);
				--m_Bindings;
				return true;
			}
		}
		return false;
	}

	//Remove all the functions bound to the delegate
	void RemoveAll()
	{
		m_Events.clear();
		m_Bindings = 0;
	}

	//Execute all functions that are bound
	void Broadcast(Args ...args) const
	{
		for (size_t i = 0; i < m_Bindings; ++i)
		{
			m_Events[i].second->Execute(args...);
		}
	}

private:
	DelegateHandle AddDelegate(DelegateHandlerPtrT&& pDelegate)
	{
		DelegateHandle handle(true);

		if (m_Bindings < m_Events.size())
		{
			m_Events[m_Bindings - 1] = std::pair<DelegateHandle, DelegateHandlerPtrT>(handle, std::move(pDelegate));
		}
		else
		{
			m_Events.push_back(std::pair<DelegateHandle, DelegateHandlerPtrT>(handle, std::move(pDelegate)));
		}
		++m_Bindings;
		return handle;
	}

	std::vector<std::pair<DelegateHandle, DelegateHandlerPtrT>> m_Events;
	int m_Bindings = 0;
};