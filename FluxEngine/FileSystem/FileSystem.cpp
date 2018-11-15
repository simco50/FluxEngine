#include "FluxEngine.h"
#include "FileSystem.h"
#include "FileSystem/MountPoint/PakMountPoint.h"
#include "FileSystem/MountPoint/PhysicalMountPoint.h"
#include "FileSystem/File/PhysicalFile.h"
#include "FileSystemHelpers.h"
#include "MountPoint/ZipMountPoint.h"

std::vector<MountPointPair> FileSystem::m_MountPoints;

std::vector<std::string> FileSystem::m_PakLocations;

FileSystem::FileSystem()
{}

FileSystem::~FileSystem()
{}

bool FileSystem::Mount(const std::string& physicalPath, const std::string& virtualPath, const ArchiveType type)
{
	AUTOPROFILE_DESC(FileSystem_Mount, physicalPath);

	std::unique_ptr<IMountPoint> pPtr = CreateMountPoint(FixPath(physicalPath), type);
	if (pPtr == nullptr)
	{
		return false;
	}

	if (!pPtr->OnMount())
	{
		return false;
	}

	m_MountPoints.push_back(MountPointPair(FixPath(virtualPath), std::move(pPtr)));

	//Sort the mountpoints depending on their priority
	std::sort(m_MountPoints.begin(), m_MountPoints.end(),
		[](const MountPointPair& a, const MountPointPair& b)
	{
		return a.second->GetOrder() > b.second->GetOrder();
	}
	);
	FLUX_LOG(Info, "[FileSystem::Mount] > Mounted '%s' on '%s'", physicalPath.c_str(), virtualPath.c_str());

	return true;
}

bool FileSystem::Mount(const std::string& physicalPath, const ArchiveType type /*= ArchiveType::Physical*/)
{
	return Mount(physicalPath, "", type);
}

void FileSystem::AddPakLocation(const std::string& path, const std::string& virtualPath)
{
	if (std::find(m_PakLocations.begin(), m_PakLocations.end(), path) != m_PakLocations.end())
		return;
	m_PakLocations.push_back(path);
	for (const std::string& location : m_PakLocations)
	{
		std::vector<std::string> pakFiles;
		GetFilesWithExtension(location, pakFiles, "pak", false);
		for (const std::string& pakFile : pakFiles)
			Mount(pakFile, virtualPath, ArchiveType::Pak);
	}
}

std::unique_ptr<File> FileSystem::GetFile(const std::string& fileName)
{
	const std::string path = Paths::Normalize(fileName);
	//Search through all the mount points
	//The points that got mounted first get prioritized
	for (const auto& pMp : m_MountPoints)
	{
		//strip out the mount point's virtual file path
		std::string searchPath = path.substr(0, pMp.first.size());
		if (pMp.first == searchPath)
		{
			std::unique_ptr<File> pFile = pMp.second->GetFile(path.substr(pMp.first.size() + 1));
			//If we didn't find the file, continue looking in the other mount points
			if (pFile == nullptr)
			{
				continue;
			}
			pFile->SetSource(fileName);
			return pFile;
		}
	}
	return nullptr;
}

DateTime FileSystem::GetLastModifiedTime(const std::string& fileName)
{
	FileAttributes attibutes;
	if (!GetFileAttributes(fileName, attibutes))
	{
		return DateTime(0);
	}
	return attibutes.ModifiedTime;
}

DateTime FileSystem::GetLastAccessTime(const std::string& fileName)
{
	FileAttributes attibutes;
	if (!GetFileAttributes(fileName, attibutes))
	{
		return DateTime(0);
	}
	return attibutes.AccessTime;
}

DateTime FileSystem::GetCreationTime(const std::string& fileName)
{
	FileAttributes attibutes;
	if (!GetFileAttributes(fileName, attibutes))
	{
		return DateTime(0);
	}
	return attibutes.CreationTime;
}

int64 FileSystem::GetFileSize(const std::string& fileName)
{
	FileAttributes attibutes;
	if (!GetFileAttributes(fileName, attibutes))
	{
		return -1;
	}
	return attibutes.Size;
}

bool FileSystem::Delete(const std::string& fileName)
{
	return DeleteFile(fileName.c_str()) == TRUE;
}

bool FileSystem::Move(const std::string& fileName, const std::string& newFileName, const bool overWrite /*= true*/)
{
	return MoveFileEx(fileName.c_str(), newFileName.c_str(), overWrite ? MOVEFILE_REPLACE_EXISTING : 0) == TRUE;
}

bool FileSystem::Copy(const std::string& fileName, const std::string& newFileName, const bool overWrite /*= true*/)
{
	return CopyFileEx(fileName.c_str(), newFileName.c_str(), nullptr, nullptr, nullptr, overWrite ? MOVEFILE_REPLACE_EXISTING : 0) == TRUE;
}

bool FileSystem::IterateDirectory(const std::string& path, FileVisitor& visitor)
{
	WIN32_FIND_DATAA find_data;
	auto handle = FindFirstFileA((path + "\\*").c_str(), &find_data);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	do
	{
		std::string filePath = find_data.cFileName;
		if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			if (filePath != "." && filePath != "..")
			{
				if (visitor.Visit(filePath, false) == false)
				{
					return true;
				}
			}
		}
		else
		{
			if (visitor.Visit(filePath, true) == false)
			{
				return true;
			}
			if (visitor.IsRecursive())
			{
				IterateDirectory(filePath, visitor);
			}
		}
	} while (FindNextFileA(handle, &find_data) != 0);
	return true;
}

bool FileSystem::GetFileAttributes(const std::string& filePath, FileAttributes& attributes)
{
	WIN32_FILE_ATTRIBUTE_DATA info;
	if (GetFileAttributesEx(filePath.c_str(), GetFileExInfoStandard, &info) == false)
	{
		return false;
	}
	SYSTEMTIME systemTime;
	FILETIME localTime;
	FileTimeToLocalFileTime(&info.ftCreationTime, &localTime);
	FileTimeToSystemTime(&localTime, &systemTime);
	attributes.CreationTime = DateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
	FileTimeToLocalFileTime(&info.ftLastWriteTime, &localTime);
	FileTimeToSystemTime(&localTime, &systemTime);
	attributes.ModifiedTime = DateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
	FileTimeToLocalFileTime(&info.ftLastAccessTime, &localTime);
	FileTimeToSystemTime(&localTime, &systemTime);
	attributes.AccessTime = DateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
	attributes.IsDirectory = info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	if (!attributes.IsDirectory)
	{
		LARGE_INTEGER size;
		size.HighPart = info.nFileSizeHigh;
		size.LowPart = info.nFileSizeLow;
		attributes.Size = (long long)size.QuadPart;
	}
	attributes.IsReadOnly = info.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
	return true;
}

void FileSystem::GetFilesInDirectory(const std::string& directory, std::vector<std::string>& files, const bool recursive)
{
	struct SimpleVisitor : public FileVisitor
	{
		SimpleVisitor(std::vector<std::string>& files, const bool recursive) :
			Files(files), Recursive(recursive)
		{}
		virtual bool Visit(const std::string& fileName, const bool isDirectory) override
		{
			if(isDirectory == false)
				Files.push_back(fileName);
			return true;
		}
		virtual bool IsRecursive() const override
		{
			return Recursive;
		}
		std::vector<std::string>& Files;
		bool Recursive;
	};
	SimpleVisitor visitor(files, recursive);
	IterateDirectory(directory, visitor);
}

void FileSystem::GetFilesWithExtension(const std::string& directory, std::vector<std::string>& files, const std::string& extension, const bool recursive)
{
	struct SimpleVisitor : public FileVisitor
	{
		SimpleVisitor(std::vector<std::string>& files, const std::string& extension, const bool recursive) :
			Files(files), pExtension(&extension), Recursive(recursive)
		{}
		virtual bool Visit(const std::string& fileName, const bool isDirectory) override
		{
			if (isDirectory == false && Paths::GetFileExtenstion(fileName) == *pExtension)
			{
				Files.push_back(fileName);
			}
			return true;
		}
		virtual bool IsRecursive() const override
		{
			return Recursive;
		}
		std::vector<std::string>& Files;
		const std::string* pExtension;
		bool Recursive;
	};
	SimpleVisitor visitor(files, extension, recursive);
	IterateDirectory(directory, visitor);
}

std::string FileSystem::FixPath(const std::string& path)
{
	std::string output;
	if (path.substr(0, 2) == "./" || path.substr(0, 2) == ".\\")
	{
		output = std::string(path.begin() + 2, path.end());
	}
	else
	{
		output = path;
	}
	std::replace(output.begin(), output.end(), '\\', '/');
	ToLower(output);
	if (output.back() == '/')
	{
		output.pop_back();
	}

	return output;
}

std::unique_ptr<IMountPoint> FileSystem::CreateMountPoint(const std::string& physicalPath, const ArchiveType type)
{
	switch (type)
	{
	case ArchiveType::Physical: return std::make_unique<PhysicalMountPoint>(physicalPath);
	case ArchiveType::Pak: return std::make_unique<PakMountPoint>(physicalPath);
	case ArchiveType::Zip: return std::make_unique<ZipMountPoint>(physicalPath);
	default:
		checkNoEntry();
	}
	return nullptr;
}

