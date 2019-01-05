#include "FluxEngine.h"
#include "File.h"

File::File(const std::string& fileName)
{
	m_Source = fileName;
}

bool File::Exists() const
{
	return Paths::FileExists(m_Source);
}

std::string File::GetDirectoryPath() const
{
	return Paths::GetDirectoryPath(m_Source);
}

std::string File::GetFileName() const
{
	return Paths::GetFileName(m_Source);
}
