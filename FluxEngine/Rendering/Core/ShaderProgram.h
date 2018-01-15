#pragma once
#include "GraphicsDefines.h"

class ShaderVariation;
struct ShaderParameter;

class ShaderProgram
{
public:
	ShaderProgram(const array<ShaderVariation*, (size_t)ShaderType::MAX> shaders);
	~ShaderProgram();

	bool SetParameter(const std::string& name, const void* pData);

private:
	map<std::string, const ShaderParameter*> m_ShaderParameters;

	array<ShaderVariation*, (size_t)ShaderType::MAX> m_CurrentShaders = {};
};