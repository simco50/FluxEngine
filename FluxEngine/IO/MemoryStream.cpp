#include "FluxEngine.h"
#include "MemoryStream.h"

size_t MemoryStream::Write(const void* pData, const size_t size)
{
	assert(m_FilePointer + size < m_Size);
	memcpy(Current(), pData, size);
	m_FilePointer += size;
	return size;
}

size_t MemoryStream::Read(void* pDestination, const size_t size)
{
	const size_t s = Math::Clamp(size, m_Size, (size_t)0);
	memcpy(pDestination, Current(), s);
	m_FilePointer += s;
	return s;
}

void* MemoryStream::Current() const
{
	return (char*)m_pBuffer + m_FilePointer;
}
