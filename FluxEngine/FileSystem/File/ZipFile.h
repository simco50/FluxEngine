#pragma once
#include "File.h"

struct ZipFileEntry;
class ZipMountPoint;

class ZipFile : public File
{
public:
	ZipFile(const std::string& fileName, ZipMountPoint* pMountPoint, ZipFileEntry* pEntry);
	virtual ~ZipFile();

	virtual bool OpenRead(bool allowWrite = false) override;
	virtual bool OpenWrite(bool append = false, bool allowRead = false) override;

	virtual size_t Read(void* pBuffer, const size_t size) override;
	virtual size_t Write(const void* pBuffer, const size_t size) override;
	bool virtual Flush() override;
	virtual bool Close() override;
	virtual bool IsOpen() const override;

	virtual bool SetPointer(const size_t position) override;

private:
	bool CacheUncompressedData();

	ZipFileEntry* m_pTableEntry;
	ZipMountPoint* m_pMountPoint;

	std::vector<char> m_UncompressedCache;
};