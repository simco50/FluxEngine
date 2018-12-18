#include "FluxEngine.h"
#include "PostProcessing.h"
#include "Core\Graphics.h"
#include "Core\RenderTarget.h"
#include "Rendering/Core/Texture2D.h"
#include "Core\Texture3D.h"
#include "Material.h"
#include "Core\DepthStencilState.h"
#include "Input\InputEngine.h"
#include "Core\BlendState.h"
#include "Camera\Camera.h"
#include "Scenegraph\SceneNode.h"
#include "Renderer.h"

PostProcessing::PostProcessing(Context* pContext)
	: Component(pContext)
{
	AUTOPROFILE(PostProcessing_Create);

	m_pGraphics = m_pContext->GetSubsystem<Graphics>();

	m_pRenderTexture = std::make_unique<Texture2D>(m_pContext);
	m_pDepthTexture = std::make_unique<Texture2D>(m_pContext);

	m_pBlitVertexShader = m_pGraphics->GetShader("Shaders/Blit", ShaderType::VertexShader);
	m_pBlitPixelShader = m_pGraphics->GetShader("Shaders/Blit", ShaderType::PixelShader);
}

PostProcessing::~PostProcessing()
{
}

void PostProcessing::Draw()
{
	if (m_Materials.empty())
	{
		return;
	}

	AUTOPROFILE(PostProcessing_Draw);

	m_pGraphics->InvalidateShaders();
	m_pGraphics->SetShader(ShaderType::VertexShader, m_pBlitVertexShader);
	m_pGraphics->SetViewport(m_pCamera->GetViewport());

	RenderTarget* pOriginalTarget = m_pGraphics->GetRenderTarget();

	RenderTarget* pCurrentSource = m_pCamera->GetRenderTarget();
	RenderTarget* pCurrentTarget = m_pRenderTexture->GetRenderTarget();
	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(false);
	m_pGraphics->GetDepthStencilState()->SetDepthWrite(false);

	int activeMaterials = 0;
	for (const auto& pMaterial : m_Materials)
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
		m_pGraphics->GetBlendState()->SetBlendMode(pMaterial.second->GetBlendMode(), false);

		m_pGraphics->SetTexture(TextureSlot::Diffuse, pCurrentSource->GetParentTexture());
		m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, 0, 3);
		std::swap(pCurrentSource, pCurrentTarget);
	}
	m_pGraphics->SetTexture(TextureSlot::Diffuse, nullptr);
	m_pGraphics->FlushSRVChanges(false);

	m_pGraphics->SetRenderTarget(0, pOriginalTarget);

	//Do an extra blit if the shader count is odd
	if (activeMaterials % 2 == 1)
	{
		m_pGraphics->SetShader(ShaderType::PixelShader, m_pBlitPixelShader);
		m_pGraphics->SetTexture(TextureSlot::Diffuse, m_pRenderTexture.get());
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

void PostProcessing::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);

	m_pCamera = pNode->GetComponent<Camera>();
	checkf(m_pCamera, "[PostProcessing::OnNodeSet] Post Processing requires a Camera component");
	OnResize(m_pCamera->GetViewport());

	Renderer* pRenderer = GetSubsystem<Renderer>();
	pRenderer->AddPostProcessing(this);
}

void PostProcessing::OnResize(const FloatRect& viewport)
{
	int msaa = m_pCamera->GetRenderTarget()->GetParentTexture()->GetMultiSample();
	m_pRenderTexture->SetSize((int)viewport.GetWidth(), (int)viewport.GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::RENDERTARGET, msaa, nullptr);
	m_pRenderTexture->SetAddressMode(TextureAddressMode::CLAMP);
	m_pDepthTexture->SetSize((int)viewport.GetWidth(), (int)viewport.GetHeight(), DXGI_FORMAT_R24G8_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, msaa, nullptr);
}