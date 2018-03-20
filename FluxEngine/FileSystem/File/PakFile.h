#pragma once
#include "File.h"

class IMountPoint;
class PakMountPoint;
struct PakFileEntry;

class PakFile : public File
{
public:
	PakFile(const std::string& fileName, PakMountPoint* pMountPoint, PakFileEntry* pEntry) :
		File(fileName), m_pTableEntry(pEntry), m_pMountPoint(pMountPoint)
	{}
	virtual ~PakFile()
	{}

	virtual bool Open(const FileMode mode) override;
	virtual size_t Read(void* pBuffer, const size_t size) override;
	virtual size_t Write(const void* pBuffer, const size_t size) override;
	bool virtual Flush() override;
	virtual bool Close() override { return true; }
	virtual bool IsOpen() const override;
	virtual size_t GetSize() const override;

	virtual bool SetPointer(const size_t position) override;

private:
	bool CacheUncompressedData();

	PakFileEntry* m_pTableEntry;
	PakMountPoint* m_pMountPoint;

	std::vector<char> m_UncompressedCache;
};