#pragma once
#include "InputStream.h"
#include "OutputStream.h"

class MemoryStream : public InputStream, public OutputStream
{
public:
	MemoryStream(void* pData, const size_t size)
		: InputStream(size), m_pBuffer(pData)
	{}

	virtual ~MemoryStream()
	{}

	virtual size_t Write(const void* pData, const size_t size) override;
	virtual size_t Read(void* pDestination, const size_t size) override;

private:
	void* Current() const;
	void* m_pBuffer;
};