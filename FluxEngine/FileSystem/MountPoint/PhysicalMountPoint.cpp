#include "FluxEngine.h"
#include "PhysicalMountPoint.h"
#include "FileSystem/File/PhysicalFile.h"
#include "FileSystem/FileSystemHelpers.h"

PhysicalMountPoint::PhysicalMountPoint(const std::string& physicalPath)
	: IMountPoint(physicalPath, 0)
{

}

PhysicalMountPoint::~PhysicalMountPoint()
{

}

bool PhysicalMountPoint::OnMount()
{
	if (!RegisterDirectory(m_PhysicalPath))
	{
		return false;
	}

	sort(m_FileEntries.begin(), m_FileEntries.end(), [](const PhysicalFileEntry& a, const PhysicalFileEntry& b) { return a.FilePath > b.FilePath; });

	return true;
}

bool PhysicalMountPoint::HasFile(const std::string& filePath) const
{
	const std::string resolvedPath = Paths::Normalize(m_PhysicalPath + "/" + filePath);
	return std::find_if(m_FileEntries.begin(), m_FileEntries.end(), [&resolvedPath](const PhysicalFileEntry& entry)
	{
		return entry.FilePath == resolvedPath;
	}) != m_FileEntries.end();
}

std::unique_ptr<File> PhysicalMountPoint::GetFile(const std::string& filePath)
{
	const std::string resolvedPath = Paths::Normalize(m_PhysicalPath + "/" + filePath);
	auto pIt = std::find_if(m_FileEntries.begin(), m_FileEntries.end(), [&resolvedPath](const PhysicalFileEntry& entry)
	{
		return strcmp(entry.FilePath.c_str(), resolvedPath.c_str()) == 0;
	});
	if (pIt == m_FileEntries.end())
	{
		return nullptr;
	}

	return std::make_unique<PhysicalFile>(resolvedPath);
}

bool PhysicalMountPoint::RegisterDirectory(const std::string& path)
{
	WIN32_FIND_DATAA find_data;
	auto handle = FindFirstFileA((path + "\\*").c_str(), &find_data);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	do
	{
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
			{
				continue;
			}
			if (!RegisterDirectory(path + "/" + find_data.cFileName))
			{
				FindClose(handle);
				return false;
			}
		}
		else
		{
			m_FileEntries.push_back(PhysicalFileEntry(Paths::Normalize(path + "/" + find_data.cFileName)));
		}
	} while (FindNextFileA(handle, &find_data) != 0);

	FindClose(handle);

	return true;
}
