#pragma once
#include "GraphicsDefines.h"

class ShaderVariation;
struct ShaderParameter;

class ShaderProgram
{
public:
	ShaderProgram(const std::array<ShaderVariation*, (size_t)ShaderType::MAX> shaders);
	~ShaderProgram();

	bool SetParameter(const std::string& name, const void* pData);

private:
	std::map<std::string, const ShaderParameter*> m_ShaderParameters;

	std::array<ShaderVariation*, (size_t)ShaderType::MAX> m_CurrentShaders = {};
};