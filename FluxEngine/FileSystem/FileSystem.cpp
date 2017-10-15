#include "stdafx.h"
#include "FileSystem.h"
#include "FileSystem/MountPoint/PakMountPoint.h"
#include "FileSystem/MountPoint/PhysicalMountPoint.h"
#include "FileSystem/File/PhysicalFile.h"
#include "FileSystemHelpers.h"

std::vector<MountPointPair> FileSystem::m_MountPoints;

FileSystem::FileSystem()
{}

FileSystem::~FileSystem()
{}

bool FileSystem::Mount(const std::string& path, const std::string& virtualPath, const ArchiveType type)
{
	std::unique_ptr<IMountPoint> pPtr = CreateMountPoint(FixPath(path), type);
	if (pPtr == nullptr)
		return false;

	if (!pPtr->OnMount())
		return false;

	m_MountPoints.push_back(MountPointPair(FixPath(virtualPath), std::move(pPtr)));
	return true;
}

bool FileSystem::Mount(const std::string& path, const ArchiveType type /*= ArchiveType::Physical*/)
{
	return Mount(path, "", type);
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
			unique_ptr<IFile> pFile = pMp.second->GetFile(path.substr(pMp.first.size() + 1));

			//If we didn't find the file, continue looking in the other mount points
			if(pFile == nullptr)
				continue;
			return std::move(pFile);
		}
	}
	return nullptr;
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

