#pragma once

struct ConfigValue
{
	ConfigValue()
	{}
	explicit ConfigValue(const std::string& value)
		: Value(value)
	{}

	std::string Value;
};

struct ConfigSection
{
	ConfigValue* GetValue(const std::string& name)
	{
		auto pPtr = Values.find(name);
		return pPtr == Values.end() ? nullptr : &pPtr->second;
	}

	std::string Name;
	std::map<std::string, ConfigValue> Values;
};

struct ConfigFile
{
	ConfigSection* GetSection(const std::string& name)
	{
		auto pPtr = Sections.find(name);
		return pPtr == Sections.end() ? nullptr : &pPtr->second;
	}

	std::string FileName;
	std::map<std::string, ConfigSection> Sections;
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

	static int GetInt(const std::string& name, const std::string& section, const int defaultValue = 0, const Type type = Type::EngineIni);
	static float GetFloat(const std::string& name, const std::string& section, const float defaultValue = 0.0f, const Type type = Type::EngineIni);
	static const std::string& GetString(const std::string& name, const std::string& section, const std::string& defaultValue = "", const Type type = Type::EngineIni);
	static bool GetBool(const std::string& name, const std::string& section, const bool defaultValue = false, const Type type = Type::EngineIni);

	static bool SetValue(const std::string& name, const std::string& section, const int value, const Type type = Type::EngineIni);
	static bool SetValue(const std::string& name, const std::string& section, const float value, const Type type = Type::EngineIni);
	static bool SetValue(const std::string& name, const std::string& section, const std::string& value, const Type type = Type::EngineIni);

	//!Writes a specific (or all) config values to the proper file
	static bool Flush(const Type t = Type::MAX_TYPES);
private:

	static ConfigValue* GetValue(const std::string& name, const std::string& section, const Type t);
	static ConfigFile* GetConfigFile(const Type t);

	static bool PopulateConfigValues(const Type t);
	static bool FlushConfigValues(const Type t);

	static std::map<Type, ConfigFile> m_Configs;
};