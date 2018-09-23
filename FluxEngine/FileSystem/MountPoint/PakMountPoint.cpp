#include "FluxEngine.h"
#include "PakMountPoint.h"
#include "FileSystem/File/PakFile.h"
#include "FileSystem/File/PhysicalFile.h"
#include "FileSystem/FileSystemHelpers.h"

PakMountPoint::PakMountPoint(const std::string& physicalPath)
	: IMountPoint(physicalPath, -1)
{
}

bool PakMountPoint::OnMount()
{
	//A pak file is a regular file so load that first
	m_pPakFile = std::make_unique<PhysicalFile>(m_PhysicalPath);
	if (!m_pPakFile->OpenRead())
	{
		return false;
	}

	//Read in the header
	if (!m_pPakFile->ReadFrom(reinterpret_cast<char*>(&m_Header), 0, sizeof(PakFileHeader)))
	{
		return false;
	}

	if (strcmp("PAK", m_Header.ID) != 0)
	{
		return false;
	}

	if (m_Header.Version != PAK_VERSION)
	{
		return false;
	}

	m_Order = m_Header.ContentVersion;

	//Read in all the table entries
	m_FileEntries.resize(m_Header.NumEntries);
	if (!m_pPakFile->ReadFrom(reinterpret_cast<char*>(m_FileEntries.data()), sizeof(PakFileHeader), m_Header.NumEntries * sizeof(PakFileEntry)))
	{
		return false;
	}

	//Set all characters to lower case
	for (PakFileEntry& entry : m_FileEntries)
	{
		std::string normalized = Paths::Normalize(entry.FilePath);
		memcpy(entry.FilePath, normalized.data(), normalized.size());
	}

	//Sort the files so searching is faster
	std::sort(m_FileEntries.begin(), m_FileEntries.end(), [](const PakFileEntry& a, const PakFileEntry& b) {return a.FilePath > b.FilePath; });

	return true;
}

bool PakMountPoint::HasFile(const std::string& filePath) const
{
	const std::string resolvedPath = ToLower(filePath);

	return std::find_if(m_FileEntries.begin(), m_FileEntries.end(), [&resolvedPath](const PakFileEntry& entry)
	{
		return entry.FilePath == resolvedPath;
	}) != m_FileEntries.end();
}

std::unique_ptr<File> PakMountPoint::GetFile(const std::string& filePath)
{
	//Even if we are sure the file created exists, we still need to do a find to create the file.
	auto pIt = std::find_if(m_FileEntries.begin(), m_FileEntries.end(), [&filePath](const PakFileEntry& entry)
	{
		return entry.FilePath == filePath;
	});
	if (pIt == m_FileEntries.end())
	{
		return nullptr;
	}
	return std::make_unique<PakFile>(pIt._Ptr->FilePath, this, pIt._Ptr);
}