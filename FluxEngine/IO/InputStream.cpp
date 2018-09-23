#include "FluxEngine.h"
#include "InputStream.h"

size_t InputStream::ReadFrom(void* pDestination, const size_t from, const size_t size)
{
	SetPointer(from);
	return Read(pDestination, size);
}

bool InputStream::ReadAllBytes(std::vector<unsigned char>& buffer)
{
	const size_t size = GetSize();
	buffer.resize(size);
	return Read(buffer.data(), buffer.size()) == size;
}

bool InputStream::SetPointer(const size_t position)
{
	m_FilePointer = position;
	return true;
}

bool InputStream::SetPointerFromEnd(const size_t position)
{
	if (position > m_Size)
	{
		return false;
	}
	m_FilePointer = m_Size - position;
	return true;
}

bool InputStream::MovePointer(const int delta)
{
	return SetPointer(m_FilePointer + delta);
}

std::string InputStream::ReadSizedString()
{
	unsigned char size;
	Read(&size, sizeof(unsigned char));
	std::string str;
	str.resize(size);
	Read(&str[0], size);
	return str;
}

std::string InputStream::ReadString()
{
	std::stringstream str;
	while (!IsEoF())
	{
		char c = ReadByte();
		if (c == '\0')
		{
			return str.str();
		}
		str << c;
	}
	return str.str();
}

int InputStream::ReadInt()
{
	int out;
	return Read(&out, sizeof(int)) == sizeof(int) ? out : 0;
}

unsigned int InputStream::ReadUInt()
{
	unsigned int out;
	return Read(&out, sizeof(unsigned int)) == sizeof(unsigned int) ? out : 0;
}

float InputStream::ReadFloat()
{
	float out;
	return Read(&out, sizeof(float)) == sizeof(float) ? out : 0.0f;
}

double InputStream::ReadDouble()
{
	double out;
	return Read(&out, sizeof(double)) == sizeof(double) ? out : 0.0;
}

char InputStream::ReadByte()
{
	char out;
	return Read(&out, sizeof(char)) == sizeof(char) ? out : 0;
}

unsigned char InputStream::ReadUByte()
{
	unsigned char out;
	return Read(&out, sizeof(unsigned char)) == sizeof(unsigned char) ? out : 0;
}

bool InputStream::GetLine(std::string& outLine, const char delimiter /*= '\n'*/)
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