#pragma once

class InputStream
{
public:
	InputStream()
		: m_Size(0), m_FilePointer(0)
	{}
	InputStream(const size_t size)
		: m_Size(size), m_FilePointer(0)
	{}
	virtual ~InputStream() {}

	virtual size_t Read(void* pDestination, const size_t size) = 0;

	size_t ReadFrom(void* pDestination, const size_t from, const size_t size);
	bool ReadAllBytes(std::vector<unsigned char>& buffer);

	virtual bool SetPointer(const size_t position);
	virtual bool SetPointerFromEnd(const size_t position);
	bool MovePointer(const int delta);

	std::string ReadSizedString();
	std::string ReadString();
	int ReadInt();
	unsigned int ReadUInt();
	float ReadFloat();
	double ReadDouble();
	char ReadByte();
	unsigned char ReadUByte();

	bool GetLine(std::string& outLine, const char delimiter = '\n');

	bool IsEoF() const { return m_FilePointer >= m_Size; }
	size_t GetSize() const { return m_Size; }
	size_t TellG() const { return m_FilePointer; }
	size_t GetPointer() const { return m_FilePointer; }
	const std::string& GetSource() const { return m_Source; }
	void SetSource(const std::string& source) {	m_Source = source; }

protected:
	size_t m_Size;
	size_t m_FilePointer;
	std::string m_Source;
};