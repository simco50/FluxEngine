#include "stdafx.h"
#include "PhysicalFile.h"

PhysicalFile::~PhysicalFile()
{
	Close();
}

bool PhysicalFile::Open(const FileMode mode)
{
	DWORD access;
	DWORD creation_disposition;

	switch (mode)
	{
	case FileMode::Read:
		access = GENERIC_READ;
		creation_disposition = OPEN_EXISTING;
		break;
	case FileMode::Write:
		access = GENERIC_WRITE;
		creation_disposition = CREATE_NEW;
		break;
	case FileMode::ReadWrite:
		access = GENERIC_READ | GENERIC_WRITE;
		creation_disposition = OPEN_EXISTING;
		break;
	default:
		return false;
	}

	m_Handle = CreateFileA(
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

bool PhysicalFile::Write(const char* pBuffer, const unsigned int size)
{
	if (m_Handle == FILE_HANDLE_INVALID)
		return false;

	DWORD bytesToWrite = (DWORD)size;
	DWORD bytesWritten = 0;
	while (bytesToWrite > 0)
	{
		if (!WriteFile(m_Handle, pBuffer, bytesToWrite, &bytesWritten, nullptr))
			return false;
		bytesToWrite -= bytesWritten;
	}
	return true;
}

bool PhysicalFile::ReadAllBytes(std::vector<char>& pBuffer)
{
	if (m_Handle == FILE_HANDLE_INVALID)
		return false;

	unsigned int size = GetSize();
	pBuffer.resize(size);

	return Read(size, pBuffer.data());
}

bool PhysicalFile::Read(const unsigned int from, const unsigned int size, char* pBuffer)
{
	if (!SetPointer(from))
		return false;
	return Read(size, pBuffer);
}

bool PhysicalFile::Read(const unsigned int size, char* pBuffer)
{
	DWORD bytesToRead = size;
	DWORD bytesRead = 0;

	while (bytesToRead > 0)
	{
		if (!ReadFile(m_Handle, pBuffer + bytesRead, bytesToRead, &bytesRead, nullptr))
			return false;
		if (bytesRead == 0)
			return false;
		bytesToRead -= bytesRead;
	}
	return true;
}

bool PhysicalFile::SetPointer(const unsigned int position)
{
	if (m_Handle == INVALID_HANDLE_VALUE)
		return false;
	if (SetFilePointer(m_Handle, position, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;
	return true;
}

bool PhysicalFile::MovePointer(const unsigned int delta)
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