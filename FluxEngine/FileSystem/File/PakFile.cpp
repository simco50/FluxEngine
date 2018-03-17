#include "FluxEngine.h"
#include "PakFile.h"
#include "FileSystem/MountPoint/PakMountPoint.h"
#include "PhysicalFile.h"

#include "Helpers/Compression.h"

bool PakFile::Open(const FileMode mode, const ContentType writeMode)
{
	m_ContentType = writeMode;
	UNREFERENCED_PARAMETER(mode); 
	return true;
}

size_t PakFile::Read(void* pBuffer, const size_t size)
{
	if (m_pMountPoint == nullptr)
		return 0;
	const PakMountPoint* pMountPoint = static_cast<const PakMountPoint*>(m_pMountPoint);

	int sizeToRead = m_FilePointer + size > m_pTableEntry->UncompressedSize ? m_pTableEntry->UncompressedSize - (unsigned int)m_FilePointer : (unsigned int)size;

	//We're at the 'virtual' EOF
	if (sizeToRead <= 0)
		return 0;

	size_t read = 0;
	if (m_pTableEntry->Compressed)
	{
		//If the file is compressed we have to cache the uncompressed data if we want to use it later
		if (!CacheUncompressedData())
			return 0;

		memcpy(pBuffer, m_UncompressedCache.data() + m_FilePointer, sizeToRead);
		read = (size_t)sizeToRead;
	}
	else
	{
		read = pMountPoint->GetPakFile()->ReadFrom(pBuffer, m_pTableEntry->Offset + m_FilePointer, (size_t)sizeToRead);
	}
	m_FilePointer += read;
	return read;
}

bool PakFile::SetPointer(const size_t position)
{
	if (position > m_pTableEntry->UncompressedSize)
		return false;
	m_FilePointer = position;
	return true;
}

size_t PakFile::Write(const void* pBuffer, const size_t size)
{
	UNREFERENCED_PARAMETER(size);
	UNREFERENCED_PARAMETER(pBuffer);
	return 0;
}

bool PakFile::Flush()
{
	return false;
}

bool PakFile::IsOpen() const
{
	if (m_pTableEntry == nullptr)
		return false;
	if (m_pMountPoint == nullptr)
		return false;
	if (m_pMountPoint->GetPakFile() == nullptr)
		return false;
	return true;
}

size_t PakFile::GetSize() const
{
	assert(m_pTableEntry);
	return m_pTableEntry->UncompressedSize;
}

bool PakFile::CacheUncompressedData()
{
	if (m_UncompressedCache.size() == 0)
	{
		if (m_pMountPoint == nullptr)
			return false;
		const PakMountPoint* pMountPoint = static_cast<const PakMountPoint*>(m_pMountPoint);

		AUTOPROFILE_DESC(PakFile_CacheUncompressedData, m_pMountPoint->GetPhysicalPath());

		std::vector<char> tempBuffer;
		tempBuffer.resize(m_pTableEntry->CompressedSize);
		if (!pMountPoint->GetPakFile()->ReadFrom(tempBuffer.data(), m_pTableEntry->Offset, m_pTableEntry->CompressedSize))
			return false;

		if (!m_pTableEntry->Compressed)
			tempBuffer.swap(m_UncompressedCache);

		m_UncompressedCache.resize(m_pTableEntry->UncompressedSize);
		if (!Compression::Decompress(tempBuffer.data(), tempBuffer.size(), m_UncompressedCache))
			return false;
	}
	return true;
}
