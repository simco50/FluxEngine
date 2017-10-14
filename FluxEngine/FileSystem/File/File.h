#pragma once

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

class IFile
{
public:
	IFile(const std::string& fileName) :
		m_FileName(fileName)
	{}
	virtual ~IFile() {}

	virtual bool Open(const FileMode mode, const ContentType writeMode) = 0;
	virtual unsigned int ReadAllBytes(std::vector<char>& pBuffer) = 0;
	virtual unsigned int Read(const unsigned int from, const unsigned int size, char* pBuffer) = 0;
	virtual unsigned int Read(const unsigned int size, char* pBuffer) = 0;
	virtual unsigned int Write(const char* pBuffer, const unsigned int size) = 0;
	bool virtual Flush() = 0;
	virtual bool Close() = 0;
	virtual bool IsOpen() const = 0;
	virtual bool SetPointer(const unsigned int position) = 0;
	virtual bool MovePointer(const int delta) = 0;
	virtual unsigned int GetSize() const = 0;
	bool GetLine(std::string& outLine, const char delimiter = '\n');

	IFile& operator<<(const std::string& text);
	IFile& operator<<(const char* pData);
	IFile& operator<<(const char value);
	IFile& operator<<(const unsigned char value);
	IFile& operator<<(const int value);
	IFile& operator<<(const unsigned int value);
	IFile& operator<<(const float value);
	IFile& operator<<(const double value);
	IFile& operator<<(const bool value);
	IFile& operator<<(IFile& (*pf)(IFile&));

	IFile& operator>>(std::string& text);
	IFile& operator>>(char*& pData);
	IFile& operator>>(char& value);
	IFile& operator>>(unsigned char& value);
	IFile& operator>>(int& value);
	IFile& operator>>(unsigned int& value);
	IFile& operator>>(float& value);
	IFile& operator>>(double& value);
	IFile& operator>>(bool& value);

	static IFile& endl(IFile& other);

	std::string ReadSizedString();

protected:
	std::string m_FileName;
	ContentType m_WriteMode;
};