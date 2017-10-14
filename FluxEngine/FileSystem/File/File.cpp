#include "stdafx.h"
#include "File.h"

bool IFile::GetLine(std::string& outLine, const char delimiter /*= '\n'*/)
{
	static const int BUFFER_SIZE = 50;
	outLine.resize(BUFFER_SIZE);
	unsigned int currLength = 0;

	for (;;)
	{
		unsigned int read = Read(1, &outLine[currLength]);

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

IFile& IFile::operator<<(const std::string& text)
{
	Write(text.data(), (unsigned int)text.size());
	return *this;
}

IFile& IFile::operator<<(const char* pData)
{
	Write(pData, (unsigned int)strlen(pData));
	return *this;
}

IFile& IFile::operator<<(const char value)
{
	Write((const char*)&value, sizeof(const char));
	return *this;
}

IFile& IFile::operator<<(const unsigned char value)
{
	Write((const char*)&value, sizeof(const unsigned char));
	return *this;
}

IFile& IFile::operator<<(const int value)
{
	Write((const char*)&value, sizeof(const int));
	return *this;
}

IFile& IFile::operator<<(const unsigned int value)
{
	Write((const char*)&value, sizeof(const unsigned int));
	return *this;
}

IFile& IFile::operator<<(const float value)
{
	Write((const char*)&value, sizeof(const float));
	return *this;
}

IFile& IFile::operator<<(const double value)
{
	Write((const char*)&value, sizeof(const double));
	return *this;
}

IFile& IFile::operator<<(const bool value)
{
	Write((const char*)&value, sizeof(const bool));
	return *this;
}

IFile& IFile::operator>>(std::string& text)
{
	GetLine(text, '\0');
	return *this;
}

IFile& IFile::operator>>(char*& pData)
{
	Read((unsigned int)strlen(pData), pData);
	return *this;
}

IFile& IFile::operator>>(char& value)
{
	Read(sizeof(char), (char*)&value);
	return *this;
}

IFile& IFile::operator>>(unsigned char& value)
{
	Read(sizeof(unsigned char), (char*)&value);
	return *this;
}

IFile& IFile::operator>>(int& value)
{
	Read(sizeof(int), (char*)&value);
	return *this;
}

IFile& IFile::operator>>(unsigned int& value)
{
	Read(sizeof(unsigned int), (char*)&value);
	return *this;
}

IFile& IFile::operator>>(float& value)
{
	Read(sizeof(float), (char*)&value);
	return *this;
}

IFile& IFile::operator>>(double& value)
{
	Read(sizeof(double), (char*)&value);
	return *this;
}

IFile& IFile::operator>>(bool& value)
{
	Read(sizeof(bool), (char*)&value);
	return *this;
}

std::string IFile::ReadSizedString()
{
	unsigned char size;
	operator>>(size);
	std::string str;
	str.resize(size);
	Read(size, &str[0]);
	return str;
}