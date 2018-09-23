#pragma once
#include "IO\IOStream.h"

class IMountPoint;

enum class FileMode
{
	Read,
	Write,
	ReadWrite,
};

enum class ContentType
{
	Text,
	Binary,
};

class File : public IOStream
{
public:
	File(const std::string& fileName);
	virtual ~File() {}

	virtual bool OpenRead(bool allowWrite = false) = 0;
	virtual bool OpenWrite(bool append = false, bool allowRead = false) = 0;

	bool virtual Flush() = 0;
	virtual bool Close() = 0;
	virtual bool IsOpen() const = 0;

	bool Exists() const;
	std::string GetDirectoryPath() const;
	std::string GetFileName() const;

protected:
	std::string m_FileName;
};