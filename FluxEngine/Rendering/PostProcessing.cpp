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
#include "Geometry.h"
#include "Core\RasterizerState.h"

PostProcessing::PostProcessing(Context* pContext)
	: Component(pContext)
{
	AUTOPROFILE(PostProcessing_Create);

	m_pGraphics = m_pContext->GetSubsystem<Graphics>();
	m_pRenderer = m_pContext->GetSubsystem<Renderer>();

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

	m_pGraphics->SetViewport(m_pCamera->GetViewport());

	RenderTarget* pOriginalTarget = m_pGraphics->GetRenderTarget();
	RenderTarget* pCurrentSource = m_pCamera->GetRenderTarget();
	RenderTarget* pCurrentTarget = m_pRenderTexture->GetRenderTarget();

	int activeMaterials = 0;
	for (const auto& pMaterial : m_Materials)
	{
		if (pMaterial.first == false)
		{
			continue;
		}
		++activeMaterials;
		if (pMaterial.second->GetShader(ShaderType::VertexShader) == nullptr)
		{
			pMaterial.second->SetShader(ShaderType::VertexShader, m_pBlitVertexShader);
		}
		m_pRenderer->Blit(pCurrentSource, pCurrentTarget, pMaterial.second);

		std::swap(pCurrentSource, pCurrentTarget);
	}

	//Do an extra blit if the shader count is odd
	if (activeMaterials % 2 == 1)
	{
		m_pRenderer->Blit(m_pRenderTexture->GetRenderTarget(), pOriginalTarget);
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