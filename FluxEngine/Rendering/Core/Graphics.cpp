#include "FluxEngine.h"
#include "Graphics.h"
#include "RenderTarget.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShaderVariation.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "Shader.h"
#include "UI/ImmediateUI.h"
#include "ShaderProgram.h"
#include "Texture2D.h"

void Graphics::InvalidateShaders()
{
	for (int i = 0; i < (int)ShaderType::MAX; i++)
	{
		SetShader((ShaderType)i, nullptr);
	}
}

void Graphics::SetScissorRect(const bool enabled, const IntRect& rect)
{
	m_pRasterizerState->SetScissorEnabled(enabled);

	if (enabled && rect != m_CurrentScissorRect)
	{
		m_CurrentScissorRect = rect;
		m_ScissorRectDirty = true;
	}
}

Shader* Graphics::GetShader(const std::string& filePath)
{
	return GetSubsystem<ResourceManager>()->Load<Shader>(filePath + m_ShaderExtension);
}

ShaderVariation* Graphics::GetShader(const std::string& filePath, const ShaderType type, const std::string& defines)
{
	Shader* pShader = GetShader(filePath);
	if (pShader)
	{
		return pShader->GetOrCreateVariation(type, defines);
	}
	return nullptr;
}

bool Graphics::UsingTessellation() const
{
#ifdef SHADER_TESSELLATION_ENABLE
	return m_CurrentShaders[(size_t)ShaderType::DomainShader] && m_CurrentShaders[(size_t)ShaderType::HullShader];
#else
	return false;
#endif
}

RenderTarget* Graphics::GetRenderTarget() const
{
	return m_CurrentRenderTargets[0] ? m_CurrentRenderTargets[0] : m_pDefaultRenderTarget->GetRenderTarget();
}

void Graphics::GetDebugInfo(unsigned int& batchCount, unsigned int& primitiveCount)
{
	batchCount = m_BatchCount;
	primitiveCount = m_PrimitiveCount;
}