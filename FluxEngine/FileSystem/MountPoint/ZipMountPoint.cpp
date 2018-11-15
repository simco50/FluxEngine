#include "FluxEngine.h"
#include "ZipMountPoint.h"
#include "FileSystem/File/ZipFile.h"
#include "FileSystem/File/PhysicalFile.h"
#include "FileSystem/FileSystemHelpers.h"

const int CentralDirectoryFileSignature = 0x02014b50;
const int EndOfCentralDirectorySignature = 0x06054b50;

ZipMountPoint::ZipMountPoint(const std::string& physicalPath)
	: IMountPoint(physicalPath, -1)
{
}

ZipMountPoint::~ZipMountPoint()
{

}

bool ZipMountPoint::OnMount()
{
	//A pak file is a regular file so load that first
	m_pZipFile = std::make_unique<PhysicalFile>(m_PhysicalPath);
	if (!m_pZipFile->OpenRead())
	{
		return false;
	}

	//We're going to try and find the central directory
	std::vector<unsigned char> searchText(64);
	m_pZipFile->SetPointer(m_pZipFile->GetSize() - searchText.size());
	m_pZipFile->Read(searchText.data(), searchText.size());
	EndOfCentralDirectoryRecord* pEndOfCentralDirectoryRecord = nullptr;
	for (int i = (int)searchText.size() - 4; i >= 0; --i)
	{
		int sig = *(int*)&searchText[i];
		if (sig == EndOfCentralDirectorySignature)
		{
			pEndOfCentralDirectoryRecord = reinterpret_cast<EndOfCentralDirectoryRecord*>(&searchText[i]);
			break;
		}
	}

	if (pEndOfCentralDirectoryRecord == nullptr)
	{
		return false;
	}

	m_pZipFile->SetPointer(pEndOfCentralDirectoryRecord->StartOffset);
	CentralDirectoryFileHeader header;

	for (int i = 0; i < pEndOfCentralDirectoryRecord->TotalEntriesOnDisk; ++i)
	{
		ZipFileEntry entry;
		m_pZipFile->Read(&header, sizeof(CentralDirectoryFileHeader));
		check(header.Signature == CentralDirectoryFileSignature);
		check(header.Compression == 0 || header.Compression == 8);

		std::vector<char> fileName(header.FileNameLength);
		m_pZipFile->Read(entry.FilePath, header.FileNameLength);
		entry.FilePath[header.FileNameLength] = '\0';

		entry.CompressedSize = header.CompressedSize;
		entry.UncompressedSize = header.UncompressedSize;
		entry.Offset = header.RelativeOffset;
		entry.Compressed = header.Compression == 8;

		ToLower(entry.FilePath);

		char junk[256];
		m_pZipFile->Read(junk, header.ExtraFieldLength);
		if (entry.FilePath[header.FileNameLength - 1] != '/')
		{
			m_FileEntries.push_back(entry);
		}
	}

	ZipFileHeader fileHeader;
	for (ZipFileEntry& entry : m_FileEntries)
	{
		m_pZipFile->SetPointer(entry.Offset);
		m_pZipFile->Read(&fileHeader, sizeof(ZipFileHeader));
		char junk[256];
		m_pZipFile->Read(junk, fileHeader.FileNameLength + fileHeader.ExtraFieldLength);
		entry.Offset = (uint32)m_pZipFile->TellG();
	}

	return true;
}

bool ZipMountPoint::HasFile(const std::string& filePath) const
{
	const std::string resolvedPath = ToLower(filePath);

	return std::find_if(m_FileEntries.begin(), m_FileEntries.end(), [&resolvedPath](const ZipFileEntry& entry)
	{
		return entry.FilePath == resolvedPath;
	}) != m_FileEntries.end();
}

std::unique_ptr<File> ZipMountPoint::GetFile(const std::string& filePath)
{
	//Even if we are sure the file created exists, we still need to do a find to create the file.
	auto pIt = std::find_if(m_FileEntries.begin(), m_FileEntries.end(), [&filePath](const ZipFileEntry& entry)
	{
		return entry.FilePath == filePath;
	});
	if (pIt == m_FileEntries.end())
	{
		return nullptr;
	}
	return std::make_unique<ZipFile>(pIt._Ptr->FilePath, this, pIt._Ptr);
}