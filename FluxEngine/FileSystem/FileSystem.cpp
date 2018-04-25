#include "FluxEngine.h"
#include "FileSystem.h"
#include "FileSystem/MountPoint/PakMountPoint.h"
#include "FileSystem/MountPoint/PhysicalMountPoint.h"
#include "FileSystem/File/PhysicalFile.h"
#include "FileSystemHelpers.h"

std::vector<MountPointPair> FileSystem::m_MountPoints;

std::vector<std::string> FileSystem::m_PakLocations;

FileSystem::FileSystem()
{}

FileSystem::~FileSystem()
{}

bool FileSystem::Mount(const std::string& path, const std::string& virtualPath, const ArchiveType type)
{
	AUTOPROFILE_DESC(FileSystem_Mount, path);

	std::unique_ptr<IMountPoint> pPtr = CreateMountPoint(FixPath(path), type);
	if (pPtr == nullptr)
		return false;

	if (!pPtr->OnMount())
		return false;

	m_MountPoints.push_back(MountPointPair(FixPath(virtualPath), std::move(pPtr)));

	//Sort the mountpoints depending on their priority
	sort(m_MountPoints.begin(), m_MountPoints.end(), 
		[](const MountPointPair& a, const MountPointPair& b)
	{
		return a.second->GetOrder() > b.second->GetOrder();
	}
	);
	FLUX_LOG(Info, "[FileSystem::Mount] > Mounted '%s' on '%s'", path.c_str(), virtualPath.c_str());

	return true;
}

bool FileSystem::Mount(const std::string& path, const ArchiveType type /*= ArchiveType::Physical*/)
{
	return Mount(path, "", type);
}

void FileSystem::AddPakLocation(const std::string& path, const std::string& virtualPath)
{
	if (find(m_PakLocations.begin(), m_PakLocations.end(), path) != m_PakLocations.end())
		return;
	m_PakLocations.push_back(path);
	for (const std::string& location : m_PakLocations)
	{
		const std::vector<std::string>& pakFiles = GetPakFilesInDirectory(location);
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
			if(pFile == nullptr)
				continue;
			return std::move(pFile);
		}
	}
	return nullptr;
}

DateTime FileSystem::GetLastModifiedTime(const std::string& fileName)
{
	DateTime creationTime, accessTime, modifiedTime;
	FileAttributes attibutes;
	if (!GetFileAttributes(fileName, attibutes))
	{
		return DateTime(0);
	}
	return attibutes.ModifiedTime;
}

bool FileSystem::GetFileAttributes(const std::string filePath, FileAttributes& attributes)
{
	WIN32_FILE_ATTRIBUTE_DATA info;
	if (GetFileAttributesEx(filePath.c_str(), GetFileExInfoStandard, &info) == false)
	{
		return false;
	}
	SYSTEMTIME systemTime;
	FileTimeToSystemTime(&info.ftCreationTime, &systemTime);
	attributes.CreationTime = DateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
	FileTimeToSystemTime(&info.ftLastWriteTime, &systemTime);
	attributes.ModifiedTime = DateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
	FileTimeToSystemTime(&info.ftLastAccessTime, &systemTime);
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
	WIN32_FIND_DATAA find_data;
	auto handle = FindFirstFileA((directory + "\\*").c_str(), &find_data);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	do
	{
		std::string path = find_data.cFileName;
		if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			if (path != "." && path != "..")
			{
				files.push_back(path);
			}
		}
		else
		{
			if (recursive)
			{
				GetFilesInDirectory(path, files, true);
			}
		}
	} while (FindNextFileA(handle, &find_data) != 0);
}

std::vector<std::string> FileSystem::GetPakFilesInDirectory(const std::string& directory)
{
	std::vector<std::string> results;
	GetFilesInDirectory(directory, results, false);
	int hits = 0;
	for (size_t i = 0; i < results.size() ; i++)
	{
		if (Paths::GetFileExtenstion(results[i]) == "pak")
		{
			std::swap(results[i], results[hits]);
			++hits;
		}
	}
	results.resize(hits);
	return results;
}

std::string FileSystem::FixPath(const std::string& path)
{
	std::string output;
	if (path.substr(0, 2) == "./" || path.substr(0, 2) == ".\\")
		output = std::string(path.begin() + 2, path.end());
	else
		output = path;
	replace(output.begin(), output.end(), '\\', '/');
	ToLower(output);
		
	return output;
}

std::unique_ptr<IMountPoint> FileSystem::CreateMountPoint(const std::string& physicalPath, const ArchiveType type)
{
	switch (type)
	{
	case ArchiveType::Physical: return std::make_unique<PhysicalMountPoint>(physicalPath);
	case ArchiveType::Pak: return std::make_unique<PakMountPoint>(physicalPath);
	}
	return nullptr;
}

