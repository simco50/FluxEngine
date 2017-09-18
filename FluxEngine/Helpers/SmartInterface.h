#pragma once

namespace Smart_COM 
{
	template<class T>
	class Unique_COM
	{
	public:
		Unique_COM()
		{}

		Unique_COM(T* pPtr)
		{
			if(pPtr != nullptr)
			{
				pPtr->Release();
				pPtr = nullptr;
			}
			m_pPtr = pPtr;
		}

		~Unique_COM() { Reset(); }

		Unique_COM& operator=(T* other)
		{
			Reset();
			m_pPtr = other;
			return *this;
		}

		Unique_COM(const Unique_COM& other) = delete;

		Unique_COM& operator=(const Unique_COM& other) = delete;

		T* Get() { return m_pPtr; }
		T* Get() const { return m_pPtr; }

		T** GetAddressOf() { return &m_pPtr; }
		T** GetAddressOf() const { return &m_pPtr; }

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

		operator bool() { return m_pPtr != nullptr; }
		operator bool() const { return m_pPtr != nullptr; }

		T** operator&()
		{
			return &m_pPtr;
		}

		T* operator*()
		{
			return m_pPtr;
		}

	private:
		T* m_pPtr = nullptr;
	};
}