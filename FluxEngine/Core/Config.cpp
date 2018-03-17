#include "FluxEngine.h"
#include "Config.h"
#include "FileSystem\File\PhysicalFile.h"

std::map<Config::Type, ConfigFile> Config::m_Configs;

Config::Config()
{
	
}

Config::~Config()
{
}

void Config::Initialize()
{
	for (int i = 0; i < (int)Type::MAX_TYPES; ++i)
	{
		PopulateConfigValues((Type)i);
	}
}

int Config::GetInt(const std::string& name, const std::string& section, const int defaultValue /*= 0*/, const Type type /*= Type::EngineIni*/)
{
	ConfigValue* pValue = GetValue(name, section, type);
	if (pValue == nullptr)
		return defaultValue;
	return stoi(pValue->Value);
}

float Config::GetFloat(const std::string& name, const std::string& section, const float defaultValue /*= 0*/, const Type type /*= Type::EngineIni*/)
{
	ConfigValue* pValue = GetValue(name, section, type);
	if (pValue == nullptr)
		return defaultValue;
	return stof(pValue->Value);
}

const std::string& Config::GetString(const std::string& name, const std::string& section, const std::string& defaultValue /*= 0*/, const Type type /*= Type::EngineIni*/)
{
	ConfigValue* pValue = GetValue(name, section, type);
	if (pValue == nullptr)
		return defaultValue;
	return pValue->Value;
}

bool Config::GetBool(const std::string& name, const std::string& section, const bool defaultValue /*= 0*/, const Type type /*= Type::EngineIni*/)
{
	ConfigValue* pValue = GetValue(name, section, type);
	if (pValue == nullptr)
		return defaultValue;
	return stoi(pValue->Value) == 1;
}

bool Config::Flush(const Type t /*= Type::MAX_TYPES*/)
{
	if (t == Type::MAX_TYPES)
	{
		for (int i = 0; i < (int)Type::MAX_TYPES; ++i)
		{
			if (!FlushConfigValues((Type)i))
				return false;
		}
	}
	else
		return FlushConfigValues(t);
	return true;
}

ConfigValue* Config::GetValue(const std::string& name, const std::string& section, const Type t)
{
	ConfigFile* pFile = GetConfigFile(t);
	if (pFile == nullptr)
		return nullptr;
	ConfigSection* pSection = pFile->GetSection(section);
	if (pSection == nullptr)
		return nullptr;
	ConfigValue* pValue = pSection->GetValue(name);
	return pValue;
}

ConfigFile* Config::GetConfigFile(const Type t)
{
	auto typeMap = m_Configs.find(t);
	return typeMap == m_Configs.end() ? nullptr : &typeMap->second;
}

bool Config::PopulateConfigValues(const Type t)
{
	std::unique_ptr<PhysicalFile> pFile;

	switch (t)
	{
	case Type::EngineIni:
		pFile = std::make_unique<PhysicalFile>(Paths::EngineIniFile);
		break;
	case Type::GameIni:
		pFile = std::make_unique<PhysicalFile>(Paths::GameIniFile);
		break;
	default:
		return false;
	}

	if (pFile->Open(FileMode::Read))
	{
		std::string line;
		std::string currentSection = "";
		while (pFile->GetLine(line))
		{
			if (line.length() == 0)
				continue;
			if (line[0] == '[')
			{
				currentSection = line.substr(1, line.length() - 2);
				m_Configs[t].Sections[currentSection].Name = currentSection;
				continue;
			}
			size_t equals = line.rfind('=');
			if (equals == std::string::npos)
				continue;
			m_Configs[t].Sections[currentSection].Values[line.substr(0, equals)].Value = line.substr(equals + 1);
		}
	}
	else
		return false;

	return true;
}

bool Config::FlushConfigValues(const Type t)
{
	std::unique_ptr<PhysicalFile> pFile;
	switch (t)
	{
	case Type::EngineIni:
		pFile = std::make_unique<PhysicalFile>(Paths::EngineIniFile);
		break;
	case Type::GameIni:
		pFile = std::make_unique<PhysicalFile>(Paths::GameIniFile);
		break;
	case Type::MAX_TYPES:
	default:
		return false;
	}
	if (!pFile->Open(FileMode::Write))
		return false;

	auto valueMap = m_Configs.find(t);
	if (valueMap == m_Configs.end())
		return false;

	for (const auto& section : valueMap->second.Sections)
	{
		std::stringstream stream;
		stream << "[" << section.second.Name << "]\n";
		for (const auto& value : section.second.Values)
		{
			stream << value.first << "=" << value.second.Value << "\n";
		}
		stream << "\n";
		std::string data = stream.str();
		pFile->Write(data.c_str(), data.size());
	}

	return pFile->Close();
}