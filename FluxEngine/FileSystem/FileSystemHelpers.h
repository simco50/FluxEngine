#pragma once

inline void ToLower(std::string& str)
{
	for (char& c : str)
	{
		c = (char)tolower(c);
	}
}

inline void ToLower(char* str)
{
	while (*str != '\0')
	{
		*str = (char)tolower(*str);
		++str;
	}
}

inline std::string ToLower(const std::string& str)
{
	std::string output = str;
	for (char& c : output)
		c = (char)tolower(c);
	return output;
}