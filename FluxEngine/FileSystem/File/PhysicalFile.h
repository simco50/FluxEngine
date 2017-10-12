#pragma once

#include "File.h"

class IMountPoint;

#define FILE_HANDLE_INVALID INVALID_HANDLE_VALUE

class PhysicalFile : public IFile
{
public:
	PhysicalFile(const std::string& fileName) : IFile(fileName)
	{}
	virtual ~PhysicalFile();

	virtual bool Open(const FileMode mode) override;
	virtual bool ReadAllBytes(std::vector<char>& pBuffer) override;
	virtual bool Read(const unsigned int from, const unsigned int size, char* pBuffer) override;
	virtual bool Read(const unsigned int size, char* pBuffer) override;
	virtual bool SetPointer(const unsigned int position) override;
	virtual bool MovePointer(const unsigned int delta) override;
	virtual unsigned int GetSize() const override;
	virtual bool Close() override;
	virtual bool IsOpen() const override;
	virtual bool Write(const char* pBuffer, const unsigned int size) override;

private:
	HANDLE m_Handle = FILE_HANDLE_INVALID;
};