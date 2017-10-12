#include "stdafx.h"
#include "File.h"

bool IFile::GetLine(std::string& outLine, const char delimiter /*= '\n'*/)
{
	stringstream str;
	char c;

	while (Read(1, &c))
	{
		if (c == delimiter)
		{
			outLine = str.str();
			return true;
		}
		str << c;
	}
	if (str.str().length() > 0)
	{
		outLine = str.str();
		return true;
	}

	return false;
}

void IFile::operator<<(const std::string& text)
{
	Write(text.data(), (unsigned int)text.size());
}

std::string IFile::ReadString()
{
	unsigned char size = Read<unsigned char>();
	std::string str;
	str.resize(size);
	Read(size, &str[0]);
	return str;
}
