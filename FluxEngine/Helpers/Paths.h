#pragma once

struct Paths
{
	static bool IsSlash(const char c);

	static std::string GetFileName(const std::string& filePath);
	static std::string GetFileNameWithoutExtension(const std::string& filePath);
	static std::string GetFileExtenstion(const std::string& filePath);
	static std::string GetDirectoryPath(const std::string& filePath);

	static const std::string ScreenshotFolder;
	static const std::string GameIniFile;
	static const std::string EngineIniFile;
};

