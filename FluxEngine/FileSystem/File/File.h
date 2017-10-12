#pragma once

class IMountPoint;

enum class FileMode
{
	Read,
	Write,
	ReadWrite,
};

class IFile
{
public:
	IFile(const std::string& fileName) :
		m_FileName(fileName)
	{}
	virtual ~IFile() {}

	virtual bool Open(const FileMode mode) = 0;
	virtual bool ReadAllBytes(std::vector<char>& pBuffer) = 0;
	virtual bool Read(const unsigned int from, const unsigned int size, char* pBuffer) = 0;
	virtual bool Read(const unsigned int size, char* pBuffer) = 0;
	virtual bool Write(const char* pBuffer, const unsigned int size) = 0;
	virtual bool Close() = 0;
	virtual bool IsOpen() const = 0;
	virtual bool SetPointer(const unsigned int position) = 0;
	virtual bool MovePointer(const unsigned int delta) = 0;
	virtual unsigned int GetSize() const = 0;
	bool GetLine(std::string& outLine, const char delimiter = '\n');

	void operator<<(const std::string& text);

	template<typename T>
	T Read();
	std::string ReadString();

protected:
	std::string m_FileName;
};

template<typename T>
T IFile::Read()
{
	T value;
	Read((unsigned int)sizeof(T), (char*)&value);
	return value;
}