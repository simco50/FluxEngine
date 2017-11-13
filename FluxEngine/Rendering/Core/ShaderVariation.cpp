#include "stdafx.h"
#include "ShaderVariation.h"
#include "Shader.h"
#include "Graphics.h"
#include "ConstantBuffer.h"

#ifdef D3D11
#include "D3D11/D3D11ShaderVariation.hpp"
#endif

ShaderVariation::ShaderVariation(Shader* pShader, const ShaderType type) :
	m_pParentShader(pShader),
	m_ShaderType(type)
{

}

ShaderVariation::~ShaderVariation()
{
	Release();
}


void ShaderVariation::Release()
{
	SafeRelease(m_pShaderObject);
}

void ShaderVariation::SetDefines(const string& defines)
{
	stringstream stream(defines);
	string define;
	while (getline(stream, define, ','))
	{
		m_Defines.push_back(define);
	}
}

void ShaderVariation::SetParameter(const std::string& name, const void* value, int size)
{
	auto pParameter = m_ShaderParameters.find(name);
	if (pParameter == m_ShaderParameters.end())
		return;
#ifdef _DEBUG
	if (size != pParameter->second.Size)
	{
		FLUX_LOG(ERROR, "[ShaderVariation::SetParameter] > Size mismatch. Parameter '%s' expected value with size '%i' but input value's size is '%i'", name.c_str(), pParameter->second.Size, size);
		return;
	}
#endif
	pParameter->second.pBuffer->SetParameter(pParameter->second.Offset, size, value);

}
