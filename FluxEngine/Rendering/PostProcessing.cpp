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
	m_pGraphics = m_pContext->GetSubsystem<Graphics>();
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
	for (Material* pMaterial : m_Materials)
	{
		m_pGraphics->SetShader(ShaderType::PixelShader, pMaterial->GetShader(ShaderType::PixelShader));
		m_pGraphics->SetTexture(TextureSlot::Diffuse, nullptr);
		m_pGraphics->PrepareDraw();
		m_pGraphics->SetRenderTarget(0, pCurrentTarget);
		for (const auto& texture : pMaterial->GetTextures())
		{
			m_pGraphics->SetTexture(texture.first, texture.second);
		}
		m_pGraphics->SetTexture(TextureSlot::Diffuse, pCurrentSource->GetRenderTexture());
		m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, 0, 3);
		std::swap(pCurrentSource, pCurrentTarget);
	}
	m_pGraphics->SetTexture(TextureSlot::Diffuse, nullptr);
	m_pGraphics->PrepareDraw();
	m_pGraphics->SetRenderTarget(0, nullptr);
	if (m_Materials.size() % 2 == 1)
	{
		m_pGraphics->SetShader(ShaderType::PixelShader, m_pBlitPixelShader);
		m_pGraphics->SetTexture(TextureSlot::Diffuse, m_pIntermediateRenderTarget->GetRenderTexture());
		m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, 0, 3);
	}
}

void PostProcessing::AddEffect(Material* pMaterial)
{
	if(pMaterial)
		m_Materials.push_back(pMaterial);
}

void PostProcessing::OnResize(const int width, const int height)
{
	RenderTargetDesc renderTargetDesc = {};
	renderTargetDesc.MultiSample = m_pGraphics->GetMultisample();
	renderTargetDesc.Width = width;
	renderTargetDesc.Height = height;
	if (!m_pIntermediateRenderTarget->Create(renderTargetDesc))
		FLUX_LOG(Error, "[PostProcessing::PostProcessing] > Failed to create render target");
}
