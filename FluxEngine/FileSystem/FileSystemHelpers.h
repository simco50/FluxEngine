#pragma once

inline void ToLower(std::string& str)
{
	for (char& c : str)
	{
		c = (char)tolower(c);
	}
}

inline std::string ToLower(const std::string& str)
{
	std::string output = str;
	for (char& c : output)
		c = (char)tolower(c);
	return output;
}