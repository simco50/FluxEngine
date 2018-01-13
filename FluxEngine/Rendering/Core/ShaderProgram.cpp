#include "FluxEngine.h"
#include "ShaderProgram.h"
#include "ShaderVariation.h"
#include "ConstantBuffer.h"

ShaderProgram::ShaderProgram(const array<ShaderVariation*, GraphicsConstants::SHADER_TYPES> shaders)
{
	for (ShaderVariation* pShader : shaders)
	{
		if (pShader == nullptr)
			continue;
		const map<std::string, ShaderParameter>& parameters = pShader->GetParameters();
		for (const auto& parameter : parameters)
		{
			m_ShaderParameters[parameter.first] = &parameter.second;
		}
	}
}

ShaderProgram::~ShaderProgram()
{

}

bool ShaderProgram::SetParameter(const std::string& name, const void* pData)
{
	auto pParameter = m_ShaderParameters.find(name);
	if (pParameter == m_ShaderParameters.end())
		return false;
	return pParameter->second->pBuffer->SetParameter(pParameter->second->Offset, pParameter->second->Size, pData);
}
