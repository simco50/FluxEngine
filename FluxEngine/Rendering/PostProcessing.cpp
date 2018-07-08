#include "FluxEngine.h"
#include "PostProcessing.h"
#include "Core\Graphics.h"
#include "Core\RenderTarget.h"
#include "Rendering/Core/Texture2D.h"
#include "Core\Texture3D.h"
#include "Material.h"
#include "Core\DepthStencilState.h"
#include "Input\InputEngine.h"

PostProcessing::PostProcessing(Context* pContext) :
	Subsystem(pContext)
{
	AUTOPROFILE(PostProcessing_Create);

	m_pGraphics = m_pContext->GetSubsystem<Graphics>();

	m_pGraphics->GetRenderTarget()->GetRenderTexture()->SetAddressMode(TextureAddressMode::CLAMP);

	m_pIntermediateRenderTarget = std::make_unique<RenderTarget>(pContext);
	OnResize(m_pGraphics->GetWindowWidth(), m_pGraphics->GetWindowHeight());

	pContext->GetSubsystem<InputEngine>()->OnWindowSizeChanged().AddRaw(this, &PostProcessing::OnResize);

	m_pBlitVertexShader = m_pGraphics->GetShader("Resources/Shaders/Blit", ShaderType::VertexShader);
	m_pBlitPixelShader = m_pGraphics->GetShader("Resources/Shaders/Blit", ShaderType::PixelShader);
}

PostProcessing::~PostProcessing()
{
}

void PostProcessing::Draw()
{
	if (m_Materials.size() == 0)
		return;

	m_pGraphics->SetShader(ShaderType::VertexShader, m_pBlitVertexShader);
	m_pGraphics->SetShader(ShaderType::GeometryShader, nullptr);

	RenderTarget* pCurrentSource = m_pGraphics->GetRenderTarget();
	RenderTarget* pCurrentTarget = m_pIntermediateRenderTarget.get();

	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(false);
	int activeMaterials = 0;
	for (std::pair<bool, Material*> pMaterial : m_Materials)
	{
		if (pMaterial.first == false)
		{
			continue;
		}
		++activeMaterials;

		m_pGraphics->SetShader(ShaderType::PixelShader, pMaterial.second->GetShader(ShaderType::PixelShader));
		m_pGraphics->SetTexture(TextureSlot::Diffuse, nullptr);
		m_pGraphics->FlushSRVChanges(false);
		m_pGraphics->SetRenderTarget(0, pCurrentTarget);
		for (const auto& texture : pMaterial.second->GetTextures())
		{
			m_pGraphics->SetTexture(texture.first, texture.second);
		}
		for (auto& parameter : pMaterial.second->GetShaderParameters())
		{
			m_pGraphics->SetShaderParameter(parameter.first, parameter.second.GetData());
		}

		m_pGraphics->SetTexture(TextureSlot::Diffuse, pCurrentSource->GetRenderTexture());
		m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, 0, 3);
		std::swap(pCurrentSource, pCurrentTarget);
	}
	m_pGraphics->SetTexture(TextureSlot::Diffuse, nullptr);
	m_pGraphics->FlushSRVChanges(false);
	m_pGraphics->SetRenderTarget(0, nullptr);

	//Do an extra blit if the shader count is odd
	if (activeMaterials % 2 == 1)
	{
		m_pGraphics->SetShader(ShaderType::PixelShader, m_pBlitPixelShader);
		m_pGraphics->SetTexture(TextureSlot::Diffuse, m_pIntermediateRenderTarget->GetRenderTexture());
		m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, 0, 3);
	}
}

void PostProcessing::AddEffect(Material* pMaterial, const bool active)
{
	if (pMaterial)
	{
		m_Materials.push_back(std::pair<bool, Material*> (active, pMaterial));
	}
}

void PostProcessing::OnResize(const int width, const int height)
{
	RenderTargetDesc renderTargetDesc = {};
	renderTargetDesc.MultiSample = m_pGraphics->GetMultisample();
	renderTargetDesc.Width = width;
	renderTargetDesc.Height = height;
	if (!m_pIntermediateRenderTarget->Create(renderTargetDesc))
	{
		FLUX_LOG(Error, "[PostProcessing::OnResize] > Failed to create render target");
	}
	m_pIntermediateRenderTarget->GetRenderTexture()->SetAddressMode(TextureAddressMode::CLAMP);
}
