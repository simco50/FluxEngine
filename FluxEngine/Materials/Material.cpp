#include "stdafx.h"
#include "Material.h"
#include "Core/Components/Transform.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Core/Shader.h"

Material::Material(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{
	m_MaterialDesc.Validate();

	LoadShader();
}

Material::~Material()
{
}

void Material::LoadShader()
{
	m_pShader = make_unique<Shader>(m_pGraphics);

	for (unsigned int i = 0; i < GraphicsConstants::SHADER_TYPES; ++i)
	{
		if ((unsigned int)m_MaterialDesc.ShaderMask & i)
		{
			m_ShaderVariations[i] = m_pShader->GetVariation((ShaderType)i);
		}
	}
}

void Material::Update()
{
	UpdateShaderVariables();
}