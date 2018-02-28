#pragma once
#include "InputStream.h"
#include "OutputStream.h"

class MemoryStream : public InputStream, public OutputStream
{
public:
	MemoryStream(void* pData, const size_t size) : InputStream(size), m_pBuffer(pData)
	{}

	virtual ~MemoryStream()
	{}

	virtual size_t Write(const void* pData, const size_t size) override
	{
		assert(m_FilePointer + size < m_Size);
		memcpy(Current(), pData, size);
		m_FilePointer += size;
		return size;
	}
	virtual size_t Read(void* pDestination, const size_t size) override
	{
		assert(m_FilePointer + size < m_Size);
		memcpy(pDestination, Current(), size);
		m_FilePointer += size;
		return size;
	}

private:
	void* Current() { return (char*)m_pBuffer + m_FilePointer; }

	void* m_pBuffer;
};