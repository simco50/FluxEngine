#pragma once

struct Paths
{
	static bool IsSlash(const char c);

	static std::string GetFileName(const std::string& filePath);
	static std::string GetFileNameWithoutExtension(const std::string& filePath);
	static std::string GetFileExtenstion(const std::string& filePath);
	static std::string GetDirectoryPath(const std::string& filePath);

	static std::string Normalize(const std::string& filePath);
	static void NormalizeInline(std::string& filePath);

	static std::string ChangeExtension(const std::string& filePath, const std::string& newExtension);

	static std::string MakeRelativePath(const std::string& basePath, const std::string& filePath);

	template<typename ...StringType>
	static std::string Combine(StringType... elements)
	{
		std::string output;
		Combine(std::vector<std::string>{elements...}, output);
		return output;
	}
	static void Combine(const std::vector<std::string>& elements, std::string& output);

	static bool FileExists(const std::string& filePath);
	static bool DirectoryExists(const std::string& filePath);

	static const std::string LogsFolder;
	static const std::string ProfilingFolder;
	static const std::string ScreenshotFolder;
	static const std::string GameIniFile;
	static const std::string EngineIniFile;
	static const std::string ResourcesFolder;
	static const std::string PakFilesFolder;
	static const std::string ShaderCacheFolder;
};

