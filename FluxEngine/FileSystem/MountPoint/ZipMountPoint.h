#pragma once
#include "MountPoint.h"

class PhysicalFile;

struct ZipFileEntry
{
	char FilePath[255];
	unsigned int UncompressedSize;
	unsigned int CompressedSize;
	unsigned int Offset;
	bool Compressed;
};

#pragma pack(push, 1)
struct ZipFileHeader
{
	int Signature;
	int16 Version;
	int16 Flags;
	int16 Compression;
	int16 FileTime;
	int16 FileDate;
	int CRC;
	int CompressedSize;
	int UncompressedSize;
	int16 FileNameLength;
	int16 ExtraFieldLength;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CentralDirectoryFileHeader
{
	int Signature;
	int16 Version;
	int16 VersionNeeded;
	int16 Flags;
	int16 Compression;
	int16 FileTime;
	int16 FileDate;

	int CRC;
	int CompressedSize;
	int UncompressedSize;
	int16 FileNameLength;
	int16 ExtraFieldLength;

	int16 FileCommentLength;
	int16 StartDiskNumber;
	int16 InternalAttributes;
	int ExternalAttributes;
	int RelativeOffset;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EndOfCentralDirectoryRecord
{
	int Signature;
	int16 DiskNumber;
	int16 DiskStart;
	int16 TotalEntriesOnDisk;
	int16 TotalEntries;
	int Size;
	int StartOffset;
	int16 CommmentLength;
};
#pragma pack(pop)

class ZipMountPoint : public IMountPoint
{
public:
	explicit ZipMountPoint(const std::string& physicalPath);
	virtual ~ZipMountPoint();

	virtual bool OnMount() override;
	virtual bool HasFile(const std::string& filePath) const override;
	virtual std::unique_ptr<File> GetFile(const std::string& filePath) override;

	PhysicalFile* GetZipFile() const { return m_pZipFile.get(); }

private:
	std::unique_ptr<PhysicalFile> m_pZipFile;
	std::vector<ZipFileEntry> m_FileEntries;
};