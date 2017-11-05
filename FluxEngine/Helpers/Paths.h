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

	static std::string GetFileNameWithoutExtension(const std::string& filePath)
	{
		std::string fileName = GetFileName(filePath);
		size_t dotPos = fileName.find('.');
		if (dotPos == string::npos)
			return fileName;
		return fileName.substr(0, dotPos);
	}

	static std::string GetFileExtenstion(const std::string& filePath)
	{
		size_t dotPos = filePath.rfind('.');
		if (dotPos == string::npos)
			return filePath;
		return filePath.substr(dotPos + 1);
	}

	static std::string GetDirectoryPath(const std::string& filePath)
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