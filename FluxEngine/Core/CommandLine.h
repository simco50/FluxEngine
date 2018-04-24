#pragma once

class CommandLine
{
public:
	static bool Parse(wchar_t* pCommandLine);
	static bool Parse(const std::string& commandLine);

	static bool GetBool(const std::string& parameter);

private:
	static std::vector<std::string> m_Parameters;
};