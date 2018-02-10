#include "FluxEngine.h"
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

void ShaderVariation::SetDefines(const std::string& defines)
{
	std::stringstream stream(defines);
	std::string define;
	while (std::getline(stream, define, ','))
	{
		m_Defines.push_back(define);
	}
}