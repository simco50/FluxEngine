#pragma once

#include "File.h"

class IMountPoint;

#ifdef PLATFORM_WINDOWS
#define FILE_HANDLE_INVALID INVALID_HANDLE_VALUE
using FileHandle = HANDLE;
#endif

class PhysicalFile : public File
{
public:
	PhysicalFile(const std::string& fileName);
	virtual ~PhysicalFile();

	virtual bool OpenRead(bool allowWrite = false) override;
	virtual bool OpenWrite(bool append = false, bool allowRead = false) override;

	virtual size_t Read(void* pBuffer, const size_t size) override;
	virtual size_t Write(const void* pBuffer, const size_t size) override;
	virtual bool Flush() override;
	virtual bool Close() override;
	virtual bool IsOpen() const override;

	virtual bool SetPointer(const size_t position) override;
	virtual bool SetPointerFromEnd(const size_t position) override;

private:
	static bool CreateDirectoryTree(const std::string& path);

	FileHandle m_Handle = FILE_HANDLE_INVALID;
};