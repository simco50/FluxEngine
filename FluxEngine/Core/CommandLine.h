#pragma once

class CommandLine
{
public:
	static bool Parse(wchar_t* pCommandLine);
	static bool Parse(const std::string& commandLine);

	static bool GetBool(const std::string& parameter);
	static const std::string& Get() { return m_CommandLine; }

private:
	static std::vector<std::string> m_Parameters;
	static std::string m_CommandLine;
};