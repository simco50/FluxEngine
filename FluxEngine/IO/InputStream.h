#pragma once

class InputStream
{
public:
	InputStream() :
		m_Size(0), m_FilePointer(0)
	{}
	InputStream(const size_t size) :
		m_Size(size), m_FilePointer(0)
	{}
	virtual ~InputStream() {}

	size_t ReadFrom(void* pDestination, const size_t from, const size_t size)
	{
		SetPointer(from);
		return Read(pDestination, size);
	}
	bool ReadAllBytes(std::vector<char>& buffer)
	{
		const size_t size = GetSize();
		buffer.resize(size);
		return Read(buffer.data(), buffer.size()) == size;
	}
	virtual size_t Read(void* pDestination, const size_t size) = 0;

	virtual bool SetPointer(const size_t position) { m_FilePointer = position; return true; }

	bool MovePointer(const int delta) { return SetPointer(m_FilePointer + delta); }

	std::string ReadSizedString()
	{
		unsigned char size;
		Read(&size, sizeof(unsigned char));
		std::string str;
		str.resize(size);
		Read(&str[0], size);
		return str;
	}

	bool GetLine(std::string& outLine, const char delimiter = '\n')
	{
		static const int BUFFER_SIZE = 50;
		outLine.resize(BUFFER_SIZE);
		unsigned int currLength = 0;

		for (;;)
		{
			size_t read = Read(&outLine[currLength], 1);

			//End of file
			if (read == 0)
			{
				outLine.resize(currLength);
				if (currLength > 0)
					return true;
				return false;
			}

			if (outLine[currLength] == '\r')
			{
				continue;
			}

			else if (outLine[currLength] == delimiter)
			{
				outLine.resize(currLength);
				return true;
			}

			++currLength;
			if (currLength >= BUFFER_SIZE)
			{
				outLine.resize(currLength + BUFFER_SIZE);
			}
		}
	}

	size_t TellG() const { return m_FilePointer; }
	size_t GetPointer() const { return m_FilePointer; }
	virtual size_t GetSize() const { return m_Size; }

	const std::string& GetSource() const { return m_Source; }
	void SetSource(const std::string& source) { m_Source = source; }

protected:
	size_t m_Size;
	size_t m_FilePointer;
	std::string m_Source;
};