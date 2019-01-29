#pragma once

class ShaderVariation;
struct ShaderParameter;

class ShaderProgram
{
public:
	explicit ShaderProgram(const std::array<ShaderVariation*, (size_t)ShaderType::MAX>& shaders);
	~ShaderProgram();

	bool SetParameter(const std::string& name, const void* pData);
	bool SetParameter(StringHash hash, const void* pData);

	const ShaderParameter* GetShaderParameter(const std::string& name) const;
	const ShaderParameter* GetShaderParameter(StringHash hash) const;

private:
	std::map<StringHash, const ShaderParameter*> m_ShaderParameters;
};