#include "stdafx.h"
#include "Paths.h"

const std::string Paths::ScreenshotFolder =		"Saved\\Screenshots";
const std::string Paths::GameIniFile =			"Config\\Game.ini";
const std::string Paths::EngineIniFile =		"Config\\Engine.ini";

bool Paths::IsSlash(const char c)
{
	if (c == '\\')
		return true;
	return c == '/';
}

std::string Paths::GetFileName(const std::string& filePath)
{
	auto it = std::find_if(filePath.rbegin(), filePath.rend(), [](const char c)
	{
		return IsSlash(c);
	});
	if (it == filePath.rend())
		return filePath;

	return filePath.substr(it.base() - filePath.begin());
}

std::string Paths::GetFileNameWithoutExtension(const std::string& filePath)
{
	std::string fileName = GetFileName(filePath);
	size_t dotPos = fileName.find('.');
	if (dotPos == string::npos)
		return fileName;
	return fileName.substr(0, dotPos);
}

std::string Paths::GetFileExtenstion(const std::string& filePath)
{
	size_t dotPos = filePath.rfind('.');
	if (dotPos == string::npos)
		return filePath;
	return filePath.substr(dotPos + 1);
}

std::string Paths::GetDirectoryPath(const std::string& filePath)
{
	auto it = std::find_if(filePath.rbegin(), filePath.rend(), [](const char c)
	{
		return IsSlash(c);
	});
	if (it == filePath.rend())
		return filePath;

	return filePath.substr(0, it.base() - filePath.begin());
}