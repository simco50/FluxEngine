#pragma once

template<class T>
class Singleton
{
public:
	template<typename ...Args>
	static void CreateInstance(Args... args)
	{
		if (!m_pObj)
			m_pObj = new T(args...);
	}

	static void DestroyInstance()
	{
		if (m_pObj)
		{
			delete m_pObj;
			m_pObj = nullptr;
		}
	}

	static T* Instance()
	{
		return m_pObj;
	}

protected:
	Singleton() {}

private:
	Singleton(const Singleton& other) {}
	void operator=(const Singleton& other) {}

	static T* m_pObj;
};

template<class T>
T* Singleton<T>::m_pObj = nullptr;