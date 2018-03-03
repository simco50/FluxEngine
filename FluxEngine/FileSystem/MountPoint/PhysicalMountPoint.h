#pragma once

#include "MountPoint.h"

class PhysicalFile;
class PakFile;

struct PhysicalFileEntry
{
	PhysicalFileEntry(const std::string& filePath):
		FilePath(filePath)
	{}
	std::string FilePath;
};

class PhysicalMountPoint : public IMountPoint
{
public:
	PhysicalMountPoint(const std::string& physicalPath);
	virtual ~PhysicalMountPoint() 
	{}

	virtual bool OnMount() override;
	virtual bool HasFile(const std::string& filePath) const override;
	virtual std::unique_ptr<File> GetFile(const std::string& filePath) override;

private:
	bool RegisterDirectory(const std::string& path);
	std::vector<PhysicalFileEntry> m_FileEntries;
};