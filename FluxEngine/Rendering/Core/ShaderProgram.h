#pragma once
#include "GraphicsDefines.h"

class ShaderVariation;
struct ShaderParameter;

class ShaderProgram
{
public:
	ShaderProgram(const array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> shaders);
	~ShaderProgram();

	bool SetParameter(const std::string& name, const void* pData);

private:
	map<std::string, const ShaderParameter*> m_ShaderParameters;

	array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> m_CurrentShaders = {};
};