#pragma once
#include "MountPoint.h"

class PakFile;
class PhysicalFile;

struct PakFileHeader
{
	char ID[4] = { "PAK" };
	char Version = 0;
	char FolderPath[100];
	char PakName[50];
	unsigned char NumEntries = 0;
};

struct PakFileEntry
{
	char FilePath[255];
	bool Compressed;
	unsigned int UncompressedSize;
	unsigned int CompressedSize;
	unsigned int Offset;
};

class PakMountPoint : public IMountPoint
{
public:
	PakMountPoint(const std::string& physicalPath) : IMountPoint(physicalPath)
	{}
	virtual ~PakMountPoint()
	{}

	virtual bool OnMount() override;
	virtual bool HasFile(const std::string& filePath) const override;
	virtual std::unique_ptr<IFile> GetFile(const std::string& filePath) override;

	PhysicalFile* GetPakFile() const { return m_pPakFile.get(); }

private:
	static const int PAK_VERSION = 1;

	std::unique_ptr<PhysicalFile> m_pPakFile;
	PakFileHeader m_Header = PakFileHeader();
	std::vector<PakFileEntry> m_FileEntries;
};