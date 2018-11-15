#include "FluxEngine.h"
#include "ZipFile.h"
#include "FileSystem/MountPoint/ZipMountPoint.h"
#include "PhysicalFile.h"
#include "Helpers/Compression.h"

ZipFile::ZipFile(const std::string& fileName, ZipMountPoint* pMountPoint, ZipFileEntry* pEntry)
	: File(fileName), m_pTableEntry(pEntry), m_pMountPoint(pMountPoint)
{

}

ZipFile::~ZipFile()
{

}

bool ZipFile::OpenRead(bool allowWrite)
{
	if (allowWrite)
	{
		return false;
	}
	m_Size = m_pTableEntry->UncompressedSize;
	return true;
}

bool ZipFile::OpenWrite(bool /*append*/, bool /*allowRead*/)
{
	return false;
}

size_t ZipFile::Read(void* pBuffer, const size_t size)
{
	if (m_pMountPoint == nullptr)
	{
		return 0;
	}

	const ZipMountPoint* pMountPoint = static_cast<const ZipMountPoint*>(m_pMountPoint);
	int sizeToRead = m_FilePointer + size > m_pTableEntry->UncompressedSize ? m_pTableEntry->UncompressedSize - (unsigned int)m_FilePointer : (unsigned int)size;

	//We're at the 'virtual' EOF
	if (sizeToRead <= 0)
	{
		return 0;
	}

	size_t read;
	if (m_pTableEntry->Compressed)
	{
		//If the file is compressed we have to cache the uncompressed data if we want to use it later
		if (!CacheUncompressedData())
		{
			return 0;
		}
		memcpy(pBuffer, m_UncompressedCache.data() + m_FilePointer, sizeToRead);
		read = (size_t)sizeToRead;
	}
	else
	{
		read = pMountPoint->GetZipFile()->ReadFrom(pBuffer, m_pTableEntry->Offset + m_FilePointer, (size_t)sizeToRead);
	}

	m_FilePointer += read;
	return read;
}

bool ZipFile::SetPointer(const size_t position)
{
	if (position > m_pTableEntry->UncompressedSize)
	{
		return false;
	}
	m_FilePointer = position;
	return true;
}

size_t ZipFile::Write(const void* /*pBuffer*/, const size_t /*size*/)
{
	throw std::exception("Function not implemented");
}

bool ZipFile::Flush()
{
	return false;
}

bool ZipFile::Close()
{
	m_UncompressedCache.clear();
	return true;
}

bool ZipFile::IsOpen() const
{
	return m_pTableEntry && m_pMountPoint && m_pMountPoint->GetZipFile();
}

bool ZipFile::CacheUncompressedData()
{
	if (m_UncompressedCache.size() == 0)
	{
		if (m_pMountPoint == nullptr)
		{
			return false;
		}
		const ZipMountPoint* pMountPoint = static_cast<const ZipMountPoint*>(m_pMountPoint);

		AUTOPROFILE_DESC(ZipFile_CacheUncompressedData, m_pMountPoint->GetPhysicalPath());

		std::vector<char> tempBuffer;
		tempBuffer.resize(m_pTableEntry->CompressedSize);
		if (!pMountPoint->GetZipFile()->ReadFrom(tempBuffer.data(), m_pTableEntry->Offset, m_pTableEntry->CompressedSize))
		{
			return false;
		}

		if (!m_pTableEntry->Compressed)
		{
			tempBuffer.swap(m_UncompressedCache);
		}

		m_UncompressedCache.resize(m_pTableEntry->UncompressedSize);
		if (!Compression::Decompress(tempBuffer.data(), tempBuffer.size(), m_UncompressedCache, false))
		{
			return false;
		}
	}
	return true;
}
