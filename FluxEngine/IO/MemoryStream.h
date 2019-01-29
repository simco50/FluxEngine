#pragma once
#include "IOStream.h"

class MemoryStream : public IOStream
{
public:
	MemoryStream()
		: IOStream(0), m_pBuffer(nullptr)
	{}

	MemoryStream(void* pData, const size_t size)
		: IOStream(size), m_pBuffer(pData)
	{}

	virtual ~MemoryStream()
	{}

	virtual size_t Write(const void* pData, const size_t size) override;
	virtual size_t Read(void* pDestination, const size_t size) override;

private:
	void* Current() const;
	void* m_pBuffer;
};