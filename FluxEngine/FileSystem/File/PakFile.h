#pragma once
#include "File.h"

class IMountPoint;
class PakMountPoint;
struct PakFileEntry;

class PakFile : public IFile
{
public:
	PakFile(const std::string& fileName, PakMountPoint* pMountPoint, PakFileEntry* pEntry) :
		IFile(fileName), m_pTableEntry(pEntry), m_pMountPoint(pMountPoint)
	{}
	virtual ~PakFile()
	{}

	virtual bool Open(const FileMode mode) override;
	virtual bool ReadAllBytes(std::vector<char>& pBuffer) override;
	virtual bool Close() override { return true; }
	virtual bool Read(const unsigned int from, const unsigned int size, char* pBuffer) override;
	virtual bool Read(const unsigned int size, char* pBuffer) override;
	virtual bool SetPointer(const unsigned int position) override;
	virtual bool MovePointer(const unsigned int delta) override;
	virtual bool Write(const char* pBuffer, const unsigned int size) override;
	virtual bool IsOpen() const override;
	virtual unsigned int GetSize() const override;

private:
	bool CacheUncompressedData();

	bool Decompress(void *pInData, size_t inDataSize, std::vector<char> &outData);
	bool Compress(void *pInData, size_t inDataSize, std::vector<char> &outData);

	PakFileEntry* m_pTableEntry;
	PakMountPoint* m_pMountPoint;

	std::vector<char> m_UncompressedCache;
	unsigned int m_FilePointer = 0;
};