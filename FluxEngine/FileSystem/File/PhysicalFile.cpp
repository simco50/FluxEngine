#include "FluxEngine.h"
#include "PhysicalFile.h"

PhysicalFile::~PhysicalFile()
{
	PhysicalFile::Close();
}

bool PhysicalFile::OpenRead(bool allowWrite)
{
	DWORD access = GENERIC_READ;
	if (allowWrite)
	{
		access |= GENERIC_WRITE;
	}
	DWORD creation_disposition = allowWrite ? CREATE_NEW : OPEN_EXISTING;

	m_Handle = CreateFile(
		m_FileName.c_str(),
		access,
		0,
		nullptr,
		creation_disposition,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (m_Handle == FILE_HANDLE_INVALID)
	{
		m_Size = 0;
		return false;
	}
	else
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(m_Handle, &fileSize) != 1)
		{
			m_Size = 0;
		}
		m_Size = (unsigned int)fileSize.QuadPart;
	}
	return true;
}

bool PhysicalFile::OpenWrite(bool append, bool allowRead)
{
	DWORD access = GENERIC_WRITE;
	if (allowRead)
	{
		access = GENERIC_READ;
	}
	else
	{
		if (!CreateDirectoryTree(m_FileName))
		{
			return false;
		}
	}
	DWORD creation_disposition = allowRead ? CREATE_NEW : CREATE_ALWAYS;

	m_Handle = CreateFile(
		m_FileName.c_str(),
		access,
		0,
		nullptr,
		creation_disposition,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (m_Handle == FILE_HANDLE_INVALID)
	{
		m_Size = 0;
		return false;
	}
	else
	{
		if (append)
		{
			if (SetPointerFromEnd(0) == false)
			{
				return false;
			}
		}
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(m_Handle, &fileSize) != 1)
		{
			m_Size = 0;
		}
		m_Size = (unsigned int)fileSize.QuadPart;
	}
	return true;
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

bool PhysicalFile::CreateDirectoryTree(const std::string& path)
{
	size_t slash = path.find('\\', 0);
	while (slash != std::string::npos)
	{
		if (slash > 1)
		{
			std::string dirToCreate = path.substr(0, slash);
			BOOL success = CreateDirectory(dirToCreate.c_str(), nullptr);
			int errorCode = Misc::GetLastErrorCode();
			if(!success && errorCode != ERROR_ALREADY_EXISTS)
			{
				std::cout << Misc::GetErrorStringFromCode(errorCode) << std::endl;
				return false;
			}
		}
		slash = path.find('\\', slash + 1);
	}
	return true;
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
		m_FilePointer += written;
		m_Size += written;
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
		m_FilePointer += read;
	}
	return size - bytesToRead;
}

bool PhysicalFile::SetPointer(const size_t position)
{
	if (m_Handle == INVALID_HANDLE_VALUE)
		return false;
	if (SetFilePointer(m_Handle, (LONG)position, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;
	m_FilePointer = position;
	return true;
}

bool PhysicalFile::SetPointerFromEnd(const size_t position)
{
	if (m_Handle == INVALID_HANDLE_VALUE)
		return false;
	if (SetFilePointer(m_Handle, (LONG)position, nullptr, FILE_END) == INVALID_SET_FILE_POINTER)
		return false;
	m_FilePointer = position;
	return true;
}
