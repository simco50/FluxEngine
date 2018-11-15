#pragma once
class IMountPoint;
class PhysicalMountPoint;
class PakMountPoint;
class File;
class PhysicalFile;
class PakFile;

enum class ArchiveType
{
	Physical,
	Pak,
	Zip,
};

using MountPointPair = std::pair<std::string, std::unique_ptr<IMountPoint>>;

struct FileAttributes
{
	DateTime CreationTime;
	DateTime AccessTime;
	DateTime ModifiedTime;
	int64 Size = -1;
	bool IsReadOnly = false;
	bool IsDirectory = false;
};

struct FileVisitor
{
	FileVisitor() = default;
	virtual ~FileVisitor() = default;

	virtual bool Visit(const std::string& fileName, const bool isDirectory) = 0;
	virtual bool IsRecursive() const = 0;
};

class FileSystem
{
public:
	FileSystem();
	~FileSystem();

	static bool Mount(const std::string& physicalPath, const ArchiveType type = ArchiveType::Physical);
	static bool Mount(const std::string& physicalPath, const std::string& virtualPath, const ArchiveType type = ArchiveType::Physical);

	static void AddPakLocation(const std::string& path, const std::string& virtualPath);

	static std::unique_ptr<File> GetFile(const std::string& fileName);

	static DateTime GetLastModifiedTime(const std::string& fileName);
	static DateTime GetLastAccessTime(const std::string& fileName);
	static DateTime GetCreationTime(const std::string& fileName);
	static int64 GetFileSize(const std::string& fileName);

	static bool Delete(const std::string& fileName);
	static bool Move(const std::string& fileName, const std::string& newFileName, const bool overWrite = true);
	static bool Copy(const std::string& fileName, const std::string& newFileName, const bool overWrite = true);

	static bool IterateDirectory(const std::string& path, FileVisitor& visitor);
	static void GetFilesInDirectory(const std::string& directory, std::vector<std::string>& files, const bool recursive);
	static void GetFilesWithExtension(const std::string& directory, std::vector<std::string>& files, const std::string& extension, const bool recursive);

private:
	static bool GetFileAttributes(const std::string& filePath, FileAttributes& attributes);

	static std::string FixPath(const std::string& path);
	static std::unique_ptr<IMountPoint> CreateMountPoint(const std::string& physicalPath, const ArchiveType type);
	static std::vector<MountPointPair> m_MountPoints;

	static std::vector<std::string> m_PakLocations;
};