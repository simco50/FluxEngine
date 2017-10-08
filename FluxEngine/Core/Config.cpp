#include "stdafx.h"
#include "Config.h"

const std::string Config::ENGINE_INI = "./Config/Engine.ini";
const std::string Config::GAME_INI = "./Config/Game.ini";
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

int Config::GetInt(const string& name, const string& section, const int defaultValue /*= 0*/, const Type type /*= Type::EngineIni*/)
{
	ConfigValue* pValue = GetValue(name, section, type);
	if (pValue == nullptr)
		return defaultValue;
	return stoi(pValue->Value);
}

float Config::GetFloat(const string& name, const string& section, const float defaultValue /*= 0*/, const Type type /*= Type::EngineIni*/)
{
	ConfigValue* pValue = GetValue(name, section, type);
	if (pValue == nullptr)
		return defaultValue;
	return stof(pValue->Value);
}

const string& Config::GetString(const string& name, const string& section, const string defaultValue /*= 0*/, const Type type /*= Type::EngineIni*/)
{
	ConfigValue* pValue = GetValue(name, section, type);
	if (pValue == nullptr)
		return defaultValue;
	return pValue->Value;
}

bool Config::GetBool(const string& name, const string& section, const bool defaultValue /*= 0*/, const Type type /*= Type::EngineIni*/)
{
	ConfigValue* pValue = GetValue(name, section, type);
	if (pValue == nullptr)
		return defaultValue;
	return stoi(pValue->Value);
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

ConfigValue* Config::GetValue(const string& name, const string& section, const Type t)
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
	ifstream str;
	switch (t)
	{
	case Type::EngineIni:
		str.open(ENGINE_INI);
		break;
	case Type::GameIni:
		str.open(GAME_INI);
		break;
	default:
		return false;
	}

	if (!str.fail())
	{
		string line;
		string currentSection = "";
		while (getline(str, line))
		{
			if (line.length() == 0)
				continue;
			if (line[0] == '[')
			{
				currentSection = line.substr(1, line.length() - 2);
				m_Configs[t].Sections[currentSection].Name = currentSection;
				continue;
			}
			unsigned int equals = (unsigned int)line.rfind('=');
			if (equals == string::npos)
			{
				FLUX_LOG(WARNING, "[Config::Config()] > Wrong config syntax '%s'", line.c_str());
				continue;
			}
			m_Configs[t].Sections[currentSection].Values[line.substr(0, equals)].Value = line.substr(equals + 1);
		}
	}
	else
		return false;

	return true;
}

bool Config::FlushConfigValues(const Type t)
{
	ofstream str;
	switch (t)
	{
	case Type::EngineIni:
		str.open(ENGINE_INI);
		break;
	case Type::GameIni:
		str.open(GAME_INI);
		break;
	case Type::MAX_TYPES:
	default:
		return false;
	}

	auto valueMap = m_Configs.find(t);
	if (valueMap == m_Configs.end())
		return false;

	for (const auto& section : valueMap->second.Sections)
	{
		str << "[" << section.second.Name << "]" << endl;
		for (const auto& value : section.second.Values)
		{
			str << value.first << "=" << value.second.Value << endl;
		}
		str << endl;
	}

	str.close();
	return true;
}