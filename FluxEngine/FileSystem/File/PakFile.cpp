#include "stdafx.h"
#include "PakFile.h"
#include "FileSystem/MountPoint/PakMountPoint.h"
#include "PhysicalFile.h"

bool PakFile::Open(const FileMode mode, const ContentType writeMode)
{
	m_WriteMode = writeMode;
	UNREFERENCED_PARAMETER(mode); 
	return true;
}

unsigned int PakFile::ReadAllBytes(std::vector<char>& pBuffer)
{
	if (!SetPointer(0))
		return 0;
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
	const PakMountPoint* pMountPoint = ((PakMountPoint*)m_pMountPoint);

	unsigned int sizeToRead = m_FilePointer + size > m_pTableEntry->UncompressedSize ? m_pTableEntry->UncompressedSize - m_FilePointer : size;

	//We're at the 'virtual' EOF
	if (sizeToRead <= 0)
		return 0;

	unsigned int read = 0;
	if (m_pTableEntry->Compressed)
	{
		//If the file is compressed we have to cache the uncompressed data if we want to use it later
		if (!CacheUncompressedData())
			return 0;

		if(m_FilePointer)

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
		const PakMountPoint* pMountPoint = ((PakMountPoint*)m_pMountPoint);

		std::vector<char> tempBuffer;
		tempBuffer.resize(m_pTableEntry->CompressedSize);
		if (!pMountPoint->GetPakFile()->Read(m_pTableEntry->Offset, m_pTableEntry->CompressedSize, tempBuffer.data()))
			return false;

		if (!m_pTableEntry->Compressed)
			tempBuffer.swap(m_UncompressedCache);

		m_UncompressedCache.resize(m_pTableEntry->UncompressedSize);
		if (!Decompress(tempBuffer.data(), tempBuffer.size(), m_UncompressedCache))
			return false;
	}
	return true;
}

bool PakFile::Decompress(void *pInData, size_t inDataSize, std::vector<char> &outData)
{
	const size_t BUFSIZE = 128 * 1024;

	z_stream strm;
	strm.zalloc = 0;
	strm.zfree = 0;
	strm.next_in = reinterpret_cast<unsigned char*>(pInData);
	strm.avail_in = (uInt)inDataSize;
	strm.next_out = reinterpret_cast<unsigned char*>(outData.data());
	strm.avail_out = BUFSIZE;

	unsigned int currSize = 0;

	inflateInit(&strm);

	while (strm.avail_in != 0)
	{
		int res = inflate(&strm, Z_NO_FLUSH);
		if (res != Z_OK && res != Z_STREAM_END)
		{
			return false;
		}
		if (strm.avail_out == 0)
		{
			currSize += BUFSIZE;
			strm.next_out = reinterpret_cast<unsigned char*>(outData.data() + currSize);
			strm.avail_out = BUFSIZE;
		}
	}

	int deflate_res = Z_OK;
	while (deflate_res == Z_OK)
	{
		if (strm.avail_out == 0)
		{
			currSize += BUFSIZE;
			strm.next_out = reinterpret_cast<unsigned char*>(outData.data() + currSize);
			strm.avail_out = BUFSIZE;
		}
		deflate_res = inflate(&strm, Z_FINISH);
	}

	if (deflate_res != Z_STREAM_END)
	{
		return false;
	}

	inflateEnd(&strm);

	return true;
}

bool PakFile::Compress(void *pInData, size_t inDataSize, std::vector<char> &outData)
{
	const size_t BUFSIZE = 128 * 1024;
	unsigned char temp_buffer[BUFSIZE];

	z_stream strm;
	strm.zalloc = 0;
	strm.zfree = 0;
	strm.next_in = reinterpret_cast<unsigned char*>(pInData);
	strm.avail_in = (uInt)inDataSize;
	strm.next_out = temp_buffer;
	strm.avail_out = BUFSIZE;

	deflateInit(&strm, Z_BEST_SPEED);

	while (strm.avail_in != 0)
	{
		int res = deflate(&strm, Z_NO_FLUSH);
		if (res != Z_OK)
		{
			return false;
		}

		if (strm.avail_out == 0)
		{
			outData.insert(outData.end(), temp_buffer, temp_buffer + BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
	}

	int deflate_res = Z_OK;
	while (deflate_res == Z_OK)
	{
		if (strm.avail_out == 0)
		{
			outData.insert(outData.end(), temp_buffer, temp_buffer + BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
		deflate_res = deflate(&strm, Z_FINISH);
	}

	if (deflate_res != Z_STREAM_END)
	{
		return false;
	}

	outData.insert(outData.end(), temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
	deflateEnd(&strm);

	return true;
}
