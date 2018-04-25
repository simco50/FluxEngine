#include "FluxEngine.h"
#include "File.h"

std::string File::GetDirectoryPath() const
{
	return Paths::GetDirectoryPath(m_FileName);
}

std::string File::GetFileName() const
{
	return Paths::GetFileName(m_FileName);
}
