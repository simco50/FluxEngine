#pragma once

struct ConfigValue
{
	ConfigValue()
	{}
	ConfigValue(const string& value) :
		Value(value)
	{}

	string Value;
};

struct ConfigSection
{
	ConfigValue* GetValue(const string& name)
	{
		auto pPtr = Values.find(name);
		return pPtr == Values.end() ? nullptr : &pPtr->second;
	}

	string Name;
	map<string, ConfigValue> Values;
};

struct ConfigFile
{
	ConfigSection* GetSection(const string& name)
	{
		auto pPtr = Sections.find(name);
		return pPtr == Sections.end() ? nullptr : &pPtr->second;
	}

	string FileName;
	map<string, ConfigSection> Sections;
};

class Config
{
public:
	Config();
	~Config();

	static void Initialize();

	enum class Type
	{
		EngineIni = 0,
		GameIni = 1,
		MAX_TYPES = 2,
	};

	static int GetInt(const string& name, const string& section, const int defaultValue = 0, const Type type = Type::EngineIni);
	static float GetFloat(const string& name, const string& section, const float defaultValue = 0.0f, const Type type = Type::EngineIni);
	static const string& GetString(const string& name, const string& section, const string& defaultValue = "", const Type type = Type::EngineIni);
	static bool GetBool(const string& name, const string& section, const bool defaultValue = false, const Type type = Type::EngineIni);

	template<typename T>
	static bool SetValue(const string& name, const string& section, const T& value, const Type type = Type::EngineIni);

	//!Writes a specific (or all) config values to the proper file
	static bool Flush(const Type t = Type::MAX_TYPES);
private:

	static ConfigValue* GetValue(const string& name, const string& section, const Type t);
	static ConfigFile* GetConfigFile(const Type t);

	static bool PopulateConfigValues(const Type t);
	static bool FlushConfigValues(const Type t);

	static map<Type, ConfigFile> m_Configs;
};

template<typename T>
bool Config::SetValue(const string& name, const string& section, const T& value, const Type type /*= Type::EngineIni*/)
{
	stringstream stream;
	stream << value;
	m_Configs[type].Sections[section].Values[name] = stream.str();
	m_Configs[type].Sections[section].Name = section;
	return true;
}
