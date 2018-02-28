#pragma once

class OutputStream
{
public:
	virtual size_t Write(const void* pData, const size_t size) = 0;

	bool WriteInt(const int value) { return Write(&value, sizeof(int)) == sizeof(int); }
	bool WriteFloat(const float value) { return Write(&value, sizeof(float)) == sizeof(float); }
	bool WriteUByte(const unsigned char value) { return Write(&value, sizeof(unsigned char)) == sizeof(unsigned char); }
	bool WriteLine(const std::string& line)
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
	bool WriteString(const std::string& string)
	{
		return Write(string.data(), string.size() + 1) == string.size() + 1;
	}
};