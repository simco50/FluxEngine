#include "FluxEngine.h"
#include "PhysicalFile.h"

PhysicalFile::~PhysicalFile()
{
	PhysicalFile::Close();
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
		nullptr,
		creation_disposition,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
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

void PhysicalFile::CreateDirectoryTree(const std::string& path)
{
	size_t slash = path.find('\\', 0);
	while (slash != std::string::npos)
	{
		std::string dirToCreate = path.substr(0, slash);
		CreateDirectory(dirToCreate.c_str(), nullptr);
		slash = path.find('\\', slash + 1);
	}
}

size_t PhysicalFile::Write(const void* pBuffer, const size_t size)
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

size_t PhysicalFile::Read(void* pBuffer, const size_t size)
{
	DWORD bytesToRead = (DWORD)size;

	while (bytesToRead > 0)
	{
		DWORD read = 0;
		if (!ReadFile(m_Handle, (char*)pBuffer + size - bytesToRead, bytesToRead, &read, nullptr))
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

bool PhysicalFile::SetPointer(const size_t position)
{
	if (m_Handle == INVALID_HANDLE_VALUE)
		return false;
	if (SetFilePointer(m_Handle, (LONG)position, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;
	return true;
}

size_t PhysicalFile::GetSize() const
{
	assert(IsOpen());
	LARGE_INTEGER fileSize;
	if (GetFileSizeEx(m_Handle, &fileSize) != 1)
		return 0;
	return (unsigned int)fileSize.QuadPart;
}