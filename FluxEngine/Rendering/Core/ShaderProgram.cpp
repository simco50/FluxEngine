#include "FluxEngine.h"
#include "ShaderProgram.h"
#include "ShaderVariation.h"
#include "ConstantBuffer.h"

ShaderProgram::ShaderProgram(const std::array<ShaderVariation*, (size_t)ShaderType::MAX>& shaders)
{
	for (ShaderVariation* pShader : shaders)
	{
		if (pShader == nullptr)
			continue;
		const std::map<StringHash, ShaderParameter>& parameters = pShader->GetParameters();
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
	return SetParameter(StringHash(name), pData);
}

bool ShaderProgram::SetParameter(StringHash hash, const void* pData)
{
	auto pParameter = m_ShaderParameters.find(hash);
	if (pParameter == m_ShaderParameters.end())
	{
		return false;
	}
	return pParameter->second->pBuffer->SetParameter(pParameter->second->Offset, pParameter->second->Size, pData);
}

const ShaderParameter* ShaderProgram::GetShaderParameter(const std::string& name) const
{
	return GetShaderParameter(StringHash(name));
}

const ShaderParameter* ShaderProgram::GetShaderParameter(StringHash hash) const
{
	auto pParameter = m_ShaderParameters.find(hash);
	if (pParameter == m_ShaderParameters.end())
	{
		return nullptr;
	}
	return pParameter->second;
}
