#include "FluxEngine.h"
#include "Graphics.h"
#include "RenderTarget.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShaderVariation.h"
#include "InputLayout.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "Shader.h"
#include "UI/ImmediateUI.h"
#include "ShaderProgram.h"
#include "Core/Window.h"

#ifdef D3D11
#include "D3D11/D3D11Graphics.hpp"
#include "D3D11/D3D11GraphicsImpl.h"
#endif

Graphics::Graphics(Context* pContext, Window* pWindow) :
	Subsystem(pContext), m_pWindow(pWindow)
{
	m_pImpl = std::make_unique<GraphicsImpl>();
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

Shader* Graphics::GetShader(const std::string filePath)
{
	return GetSubsystem<ResourceManager>()->Load<Shader>(filePath);
}

ShaderVariation* Graphics::GetShader(const std::string filePath, const ShaderType type, const std::string& defines)
{
	Shader* pShader = GetShader(filePath);
	if (pShader)
		return pShader->GetVariation(type, defines);
	return nullptr;
}

void Graphics::GetDebugInfo(unsigned int& batchCount, unsigned int& primitiveCount)
{
	batchCount = m_BatchCount;
	primitiveCount = m_PrimitiveCount;
}

bool Graphics::SetShaderParameter(const std::string& name, const void* pData)
{
	if (m_ShaderProgramDirty)
	{
		unsigned int hash = 0;
		for (ShaderVariation* pVariation : m_CurrentShaders)
		{
			hash <<= 8;
			if (pVariation == nullptr)
				continue;
			hash |= pVariation->GetName().size();
		}
		auto pIt = m_ShaderPrograms.find(hash);
		if (pIt != m_ShaderPrograms.end())
			m_pCurrentShaderProgram = pIt->second.get();
		else
		{
			AUTOPROFILE(Graphics_SetShaderParameter_CreateShaderProgram);
			std::unique_ptr<ShaderProgram> pShaderProgram = std::make_unique<ShaderProgram>(m_CurrentShaders);
			m_ShaderPrograms[hash] = std::move(pShaderProgram);
			m_pCurrentShaderProgram = m_ShaderPrograms[hash].get();
		}
		m_ShaderProgramDirty = false;
	}
	return m_pCurrentShaderProgram->SetParameter(name, pData);
}