#include "stdafx.h"
#include "PhysicalFile.h"

PhysicalFile::~PhysicalFile()
{
	Close();
}

bool PhysicalFile::Open(const FileMode mode, const ContentType writeMode)
{
	m_ContentType = writeMode;

	DWORD access;
	DWORD creation_disposition;

	switch (mode)
	{
	case FileMode::Read:
		access = GENERIC_READ;
		creation_disposition = OPEN_EXISTING;
		break;
	case FileMode::Write:
	{
		access = GENERIC_WRITE;
		creation_disposition = CREATE_ALWAYS;

		CreateDirectoryTree(m_FileName);
		break;
	}
	case FileMode::ReadWrite:
		access = GENERIC_READ | GENERIC_WRITE;
		creation_disposition = CREATE_NEW;

		CreateDirectoryTree(m_FileName);
		break;
	default:
		return false;
	}

	m_Handle = CreateFile(
		m_FileName.c_str(),
		access,
		0,
		NULL,
		creation_disposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return m_Handle != FILE_HANDLE_INVALID;
}

bool PhysicalFile::Close()
{
	if (m_Handle != FILE_HANDLE_INVALID)
	{
		CloseHandle(m_Handle);
		m_Handle = FILE_HANDLE_INVALID;
	}
	return m_Handle == FILE_HANDLE_INVALID;
}

bool PhysicalFile::IsOpen() const
{
	return m_Handle != INVALID_HANDLE_VALUE;
}

void PhysicalFile::CreateDirectoryTree(const string& path)
{
	size_t slash = path.find('\\', 0);
	while (slash != string::npos)
	{
		string dirToCreate = path.substr(0, slash);
		CreateDirectory(dirToCreate.c_str(), 0);
		slash = path.find('\\', slash + 1);
	}
}

unsigned int PhysicalFile::Write(const char* pBuffer, const unsigned int size)
{
	if (m_Handle == FILE_HANDLE_INVALID)
		return 0;

	DWORD bytesToWrite = (DWORD)size;
	while (bytesToWrite > 0)
	{
		DWORD written = 0;
		if (!WriteFile(m_Handle, pBuffer, bytesToWrite, &written, nullptr))
			return 0;
		bytesToWrite -= written;
	}
	return size - bytesToWrite;
}

bool PhysicalFile::Flush()
{
	if (m_Handle == INVALID_HANDLE_VALUE)
		return false;
	return FlushFileBuffers(m_Handle) > 1;
}

unsigned int PhysicalFile::ReadAllBytes(std::vector<char>& pBuffer)
{
	if (m_Handle == FILE_HANDLE_INVALID)
		return false;

	AUTOPROFILE_DESC(PhysicalFile_ReadAllBytes, m_FileName);

	unsigned int size = GetSize();
	pBuffer.resize(size);

	return Read(size, pBuffer.data());
}

unsigned int PhysicalFile::Read(const unsigned int from, const unsigned int size, char* pBuffer)
{
	if (!SetPointer(from))
		return false;
	return Read(size, pBuffer);
}

unsigned int PhysicalFile::Read(const unsigned int size, char* pBuffer)
{
	DWORD bytesToRead = size;

	while (bytesToRead > 0)
	{
		DWORD read = 0;
		if (!ReadFile(m_Handle, pBuffer + size - bytesToRead, bytesToRead, &read, nullptr))
		{
			return size - bytesToRead;
		}
		else if (read == 0)
		{
			//EOF
			return size - bytesToRead;
		}
		bytesToRead -= read;
	}
	return size - bytesToRead;
}

bool PhysicalFile::SetPointer(const unsigned int position)
{
	if (m_Handle == INVALID_HANDLE_VALUE)
		return false;
	if (SetFilePointer(m_Handle, position, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;
	return true;
}

bool PhysicalFile::MovePointer(const int delta)
{
	if (m_Handle == INVALID_HANDLE_VALUE)
		return false;
	if (SetFilePointer(m_Handle, delta, 0, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
		return false;
	return true;
}

unsigned int PhysicalFile::GetSize() const
{
	assert(IsOpen());
	LARGE_INTEGER fileSize;
	if (GetFileSizeEx(m_Handle, &fileSize) != 1)
		return 0;
	return (unsigned int)fileSize.QuadPart;
}