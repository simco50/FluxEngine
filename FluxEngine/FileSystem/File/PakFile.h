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
	virtual unsigned int ReadAllBytes(std::vector<char>& pBuffer) override;
	virtual unsigned int Read(const unsigned int from, const unsigned int size, char* pBuffer) override;
	virtual unsigned int Read(const unsigned int size, char* pBuffer) override;
	virtual unsigned int Write(const char* pBuffer, const unsigned int size) override;
	bool virtual Flush() override;
	virtual bool Close() override { return true; }
	virtual bool SetPointer(const unsigned int position) override;
	virtual bool MovePointer(const int delta) override;
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