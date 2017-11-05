#pragma once

struct Paths
{
	static bool IsSlash(const char c)
	{
		if (c == '\\')
			return true;
		return c == '/';
	}

	static std::string GetFileName(const std::string& filePath)
	{
		auto it = std::find_if(filePath.rbegin(), filePath.rend(), [](const char c) 
		{
			return IsSlash(c);
		});
		if (it == filePath.rend())
			return filePath;
		
		return filePath.substr(it.base() - filePath.begin());
	}

	static std::string GetDirectoryName(const std::string& filePath)
	{
		auto it = std::find_if(filePath.rbegin(), filePath.rend(), [](const char c)
		{
			return IsSlash(c);
		});
		if (it == filePath.rend())
			return filePath;

		return filePath.substr(0, it.base() - filePath.begin());
	}
};