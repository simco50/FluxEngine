#pragma once

template<class T>
class Singleton
{
public:
	static T& Instance()
	{
		static T instance;
		return instance;
	}

protected:
	Singleton() {}

private:
	Singleton(const Singleton& other) {}
	void operator=(const Singleton& other) {}
};