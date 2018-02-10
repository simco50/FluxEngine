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
	FLUX_LOG(INFO, "[FileSystem::Mount] > Mounted '%s' on '%s'", path.c_str(), virtualPath.c_str());

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

std::unique_ptr<IFile> FileSystem::GetFile(const std::string& fileName)
{
	const std::string path = FixPath(fileName);
	//Search through all the mount points
	//The points that got mounted first get prioritized
	for (const auto& pMp : m_MountPoints)
	{
		//strip out the mount point's virtual file path
		std::string searchPath = path.substr(0, pMp.first.size());
		if (pMp.first == searchPath)
		{
			std::unique_ptr<IFile> pFile = pMp.second->GetFile(path.substr(pMp.first.size() + 1));

			//If we didn't find the file, continue looking in the other mount points
			if(pFile == nullptr)
				continue;
			return std::move(pFile);
		}
	}
	return nullptr;
}

//#todo: Make abstraction for directory
std::vector<std::string> FileSystem::GetPakFilesInDirectory(const std::string& directory)
{
	std::vector<std::string> results;
	WIN32_FIND_DATAA find_data;
	auto handle = FindFirstFileA((directory + "\\*").c_str(), &find_data);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return results;
	}
	do
	{
		if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			if (Paths::GetFileExtenstion(find_data.cFileName) == "pak")
				results.push_back(find_data.cFileName);
		}
	} while (FindNextFileA(handle, &find_data) != 0);
	return results;
}

std::string FileSystem::FixPath(const std::string& path)
{
	std::string output;
	if (path.substr(0, 2) == "./")
		output = std::string(path.begin() + 2, path.end());
	else
		output = path;
	replace(output.begin(), output.end(), '/', '\\');
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

