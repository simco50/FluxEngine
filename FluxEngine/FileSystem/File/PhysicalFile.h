#pragma once

#include "File.h"

class IMountPoint;

#ifdef PLATFORM_WINDOWS
#define FILE_HANDLE_INVALID INVALID_HANDLE_VALUE
using FileHandle = HANDLE;
#elif defined(PLATFORM_LINUX)
#define FILE_HANDLE_INVALID (-1)
using FileHandle = int;
#endif

class PhysicalFile : public File
{
public:
	PhysicalFile(const std::string& fileName) : File(fileName)
	{}
	virtual ~PhysicalFile();

	virtual bool Open(const FileMode mode) override;
	virtual size_t Read(void* pBuffer, const size_t size) override;
	virtual size_t Write(const void* pBuffer, const size_t size) override;
	virtual bool Flush() override;
	virtual bool Close() override;
	virtual bool IsOpen() const override;

	virtual bool SetPointer(const size_t position) override;

private:
	void CreateDirectoryTree(const std::string& path);

	FileHandle m_Handle = FILE_HANDLE_INVALID;
};