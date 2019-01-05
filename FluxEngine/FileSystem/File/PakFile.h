#pragma once
#include "File.h"

class IMountPoint;
class PakMountPoint;
struct PakFileEntry;

class PakFile : public File
{
public:
	PakFile(const std::string& fileName, PakMountPoint* pMountPoint, PakFileEntry* pEntry);
	virtual ~PakFile();

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

	PakFileEntry* m_pTableEntry;
	PakMountPoint* m_pMountPoint;

	std::vector<char> m_UncompressedCache;
};