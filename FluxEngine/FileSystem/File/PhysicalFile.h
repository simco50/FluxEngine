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

class PhysicalFile : public IFile
{
public:
	PhysicalFile(const std::string& fileName) : IFile(fileName)
	{}
	virtual ~PhysicalFile();

	virtual bool Open(const FileMode mode, const ContentType writeMode = ContentType::Text) override;
	virtual unsigned int ReadAllBytes(std::vector<char>& pBuffer) override;
	virtual unsigned int Read(const unsigned int from, const unsigned int size, char* pBuffer) override;
	virtual unsigned int Read(const unsigned int size, char* pBuffer) override;
	virtual unsigned int Write(const char* pBuffer, const unsigned int size) override;
	virtual bool Flush() override;
	virtual bool SetPointer(const unsigned int position) override;
	virtual bool MovePointer(const int delta) override;
	virtual unsigned int GetSize() const override;
	virtual bool Close() override;
	virtual bool IsOpen() const override;

private:
	void CreateDirectoryTree(const string& path);

	FileHandle m_Handle = FILE_HANDLE_INVALID;
};