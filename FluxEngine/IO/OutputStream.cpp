#include "FluxEngine.h"
#include "OutputStream.h"

bool OutputStream::WriteLine(const std::string& line)
{
	bool success = true;
	if (!Write(line.c_str(), line.size()))
		success = false;
	if (!WriteUByte(13))
		success = false;
	if (!WriteUByte(10))
		success = false;
	return success;
}

bool OutputStream::WriteString(const std::string& string)
{
	return Write(string.data(), string.size() + 1) == string.size() + 1;
}

bool OutputStream::WriteSizedString(const std::string& string)
{
	if (!WriteUByte((unsigned char)string.size()))
	{
		return false;
	}
	if(!Write(string.data(), string.size()))
	{
		return false;
	}
	return true;
}
