#pragma once

class SimpleMemory
{
public:
	SimpleMemory()
		: m_pData(nullptr), m_Size(0)
	{}

	SimpleMemory(const int size)
		: m_pData(new char[size]), m_Size(size)
	{
		memset(m_pData, 0, size);
	}

	~SimpleMemory()
	{
		Release();
	}

	void Release()
	{
		if (m_pData)
		{
			m_Size = 0;
			delete[] m_pData;
		}
	}

	void SetData(const void* pData, int offset, int size)
	{
		memcpy(m_pData + offset, pData, size);
	}

	int GetSize() const
	{
		return m_Size;
	}

	void SetSize(int size)
	{
		char* pNewData = new char[size];
		memset(pNewData, 0, size);
		if (m_pData != nullptr)
		{
			memcpy(pNewData, m_pData, m_Size < size ? m_Size : size);
			delete[] m_pData;
		}
		m_pData = pNewData;
		m_Size = size;
	}

	operator bool()
	{
		return m_pData != nullptr;
	}

	const char* GetData() const { return m_pData; }
	char* GetData() { return m_pData; }

private:
	char* m_pData;
	int m_Size;
};