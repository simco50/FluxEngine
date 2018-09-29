#pragma once

class OutputStream
{
public:
	OutputStream() = default;
	virtual ~OutputStream() = default;

	virtual size_t Write(const void* pData, const size_t size) = 0;

	bool WriteInt(const int value) { return Write(&value, sizeof(int)) == sizeof(int); }
	bool WriteFloat(const float value) { return Write(&value, sizeof(float)) == sizeof(float); }
	bool WriteFloat(const double value) { return Write(&value, sizeof(double)) == sizeof(double); }
	bool WriteByte(const char value) { return Write(&value, sizeof(char)) == sizeof(char); }
	bool WriteUByte(const unsigned char value) { return Write(&value, sizeof(unsigned char)) == sizeof(unsigned char); }
	bool WriteString(const std::string& string);
	bool WriteSizedString(const std::string& string);

	bool WriteLine(const std::string& line);
};