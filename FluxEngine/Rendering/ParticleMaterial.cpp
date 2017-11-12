#include "stdafx.h"
#include "ParticleMaterial.h"
#include "Rendering/ParticleSystem\ParticleSystem.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Core/Graphics.h"

ParticleMaterial::ParticleMaterial(Graphics* pGraphics) : Material(pGraphics)
{
	/*m_MaterialDesc.RenderQueue = RenderQueueID::ID_TRANSPARANT;
	m_MaterialDesc.ShaderFilePath = "Resources/Shaders/Particles.hlsl";
	m_MaterialDesc.ShaderMask |= ShaderType::VertexShader;
	m_MaterialDesc.ShaderMask |= ShaderType::PixelShader;
	m_MaterialDesc.ShaderMask |= ShaderType::GeometryShader;*/
}

ParticleMaterial::~ParticleMaterial()
{
}

void ParticleMaterial::SetBlendMode(const ParticleBlendMode mode)
{
	m_BlendMode = mode;
}

void ParticleMaterial::SetTexture(const string& path)
{
	UNREFERENCED_PARAMETER(path);
	m_pTexture = nullptr;
}
