#include "FluxEngine.h"
#include "CommandLine.h"

std::vector<std::string> CommandLine::m_Parameters;
std::string CommandLine::m_CommandLine;

bool CommandLine::Parse(wchar_t* pCommandLine)
{
	std::wstring temp(pCommandLine);
	return Parse(std::string(temp.begin(), temp.end()));
}

bool CommandLine::Parse(const std::string& commandLine)
{
	m_CommandLine = commandLine;
	m_Parameters.clear();
	bool quoted = false;
	bool skipArgument = false;

	int commandStart = 0;
	for (size_t i = 0; i < commandLine.size(); i++)
	{
		if (commandLine[i] == '\"')
		{
			quoted = !quoted;
		}
		else if (commandLine[i] == ' ' && !quoted)
		{
			if (!skipArgument)
			{
				m_Parameters.push_back(commandLine.substr(commandStart, i - commandStart));
			}
			skipArgument = false;
			commandStart = (int)i + 1;
		}
	}
	if (skipArgument == false)
	{
		m_Parameters.push_back(commandLine.substr(commandStart));
	}
	for (std::string& argument : m_Parameters)
	{
		argument.erase(std::remove(argument.begin(), argument.end(), '\"'), argument.end());
	}
	std::sort(m_Parameters.begin(), m_Parameters.end());
	return true;
}

bool CommandLine::GetBool(const std::string& parameter)
{
	return std::binary_search(m_Parameters.begin(), m_Parameters.end(), parameter);
}