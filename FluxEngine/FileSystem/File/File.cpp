#include "FluxEngine.h"
#include "File.h"

File& File::operator<<(const std::string& text)
{
	Write(text.data(), text.size());
	return *this;
}

File& File::operator>>(std::string& text)
{
	GetLine(text, '\0');
	return *this;
}

std::string File::GetDirectoryPath() const
{
	size_t slashIdx = m_FileName.rfind('\\');
	if (slashIdx == std::string::npos)
		return "";
	return m_FileName.substr(0, slashIdx);
}

std::string File::GetFileName() const
{
	size_t slashIdx = m_FileName.rfind('\\');
	if (slashIdx == std::string::npos)
		return m_FileName;
	return m_FileName.substr(slashIdx + 1);
}
