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
	assert(m_FilePointer + size < m_Size);
	memcpy(pDestination, Current(), size);
	m_FilePointer += size;
	return size;
}