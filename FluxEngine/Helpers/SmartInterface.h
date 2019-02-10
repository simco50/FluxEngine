#pragma once

template<class T>
class ComPtr
{
public:
	ComPtr()
	{}

	explicit ComPtr(T* pPtr)
	{
		if(pPtr != nullptr)
		{
			pPtr->Release();
			pPtr = nullptr;
		}
		m_pPtr = pPtr;
	}

	~ComPtr() { Reset(); }

	ComPtr& operator=(T* other)
	{
		Reset();
		m_pPtr = other;
		return *this;
	}

	ComPtr(const ComPtr& other) = delete;
	ComPtr& operator=(const ComPtr& other) = delete;

	ComPtr(ComPtr&& other)
		: m_pPtr(other.m_pPtr)
	{
		other.m_pPtr = nullptr;
	}

	ComPtr& operator=(ComPtr&& other)
	{
		std::swap(m_pPtr, other.m_pPtr);
		return *this;
	}

	T* Get() { return m_pPtr; }
	T* Get() const { return m_pPtr; }

	T** GetAddressOf() { return &m_pPtr; }
	T* const * GetAddressOf() const { return &m_pPtr; }

	T* operator->() { return m_pPtr; }
	T* operator->() const { return m_pPtr; }

	bool IsValid()
	{
		return m_pPtr != nullptr;
	}

	void Reset()
	{
		if (m_pPtr)
		{
			m_pPtr->Release();
			m_pPtr = nullptr;
		}
	}

	T* Move()
	{
		T* ptr = m_pPtr;
		m_pPtr = nullptr;
		return ptr;
	}

	T* Release()
	{
		T* pPtr = m_pPtr;
		m_pPtr = nullptr;
		return pPtr;
	}

	operator bool() { return m_pPtr != nullptr; }
	operator bool() const { return m_pPtr != nullptr; }

	T* operator*() const
	{
		return m_pPtr;
	}

private:
	T* m_pPtr = nullptr;
};
