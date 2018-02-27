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

unsigned int PakFile::ReadAllBytes(std::vector<char>& pBuffer)
{
	if (!SetPointer(0))
		return 0;

	AUTOPROFILE_DESC(PhysicalFile_ReadAllBytes, m_FileName);

	pBuffer.resize(m_pTableEntry->UncompressedSize);
	return Read((unsigned int)pBuffer.size(), pBuffer.data());
}

unsigned int PakFile::Read(const unsigned int from, const unsigned int size, char* pBuffer)
{
	if (!SetPointer(from))
		return 0;
	return Read(size, pBuffer);
}

unsigned int PakFile::Read(const unsigned int size, char* pBuffer)
{
	if (m_pMountPoint == nullptr)
		return 0;
	const PakMountPoint* pMountPoint = static_cast<const PakMountPoint*>(m_pMountPoint);

	int sizeToRead = m_FilePointer + size > m_pTableEntry->UncompressedSize ? m_pTableEntry->UncompressedSize - m_FilePointer : size;

	//We're at the 'virtual' EOF
	if (sizeToRead <= 0)
		return 0;

	unsigned int read;
	if (m_pTableEntry->Compressed)
	{
		//If the file is compressed we have to cache the uncompressed data if we want to use it later
		if (!CacheUncompressedData())
			return 0;

		memcpy(pBuffer, m_UncompressedCache.data() + m_FilePointer, sizeToRead);
		read = sizeToRead;
	}
	else
	{
		read = pMountPoint->GetPakFile()->Read(m_pTableEntry->Offset + m_FilePointer, sizeToRead, pBuffer);
	}
	if (!MovePointer(sizeToRead))
		return 0;
	return read;
}

bool PakFile::SetPointer(const unsigned int position)
{
	if (position >= m_pTableEntry->UncompressedSize)
		return false;
	m_FilePointer = position;
	return true;
}

bool PakFile::MovePointer(const int delta)
{
	if (m_FilePointer + delta > m_pTableEntry->UncompressedSize)
		return false;
	m_FilePointer += delta;
	return true;
}

unsigned int PakFile::Write(const char* pBuffer, const unsigned int size)
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

unsigned int PakFile::GetSize() const
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
		if (!pMountPoint->GetPakFile()->Read(m_pTableEntry->Offset, m_pTableEntry->CompressedSize, tempBuffer.data()))
			return false;

		if (!m_pTableEntry->Compressed)
			tempBuffer.swap(m_UncompressedCache);

		m_UncompressedCache.resize(m_pTableEntry->UncompressedSize);
		if (!Compression::Decompress(tempBuffer.data(), tempBuffer.size(), m_UncompressedCache))
			return false;
	}
	return true;
}
