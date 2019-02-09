#include "FluxEngine.h"
#include "PostProcessing.h"
#include "Core\Graphics.h"
#include "Core\RenderTarget.h"
#include "Rendering/Core/Texture2D.h"
#include "Core\Texture3D.h"
#include "Material.h"
#include "Input\InputEngine.h"
#include "Camera\Camera.h"
#include "Scenegraph\SceneNode.h"
#include "Renderer.h"
#include "Geometry.h"
#include "Core\CommandContext.h"

PostProcessing::PostProcessing(Context* pContext)
	: Component(pContext)
{
	AUTOPROFILE(PostProcessing_Create);

	m_pGraphics = GetSubsystem<Graphics>();
	m_pRenderer = GetSubsystem<Renderer>();

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

	GraphicsCommandContext* pCommandContext = m_pGraphics->GetGraphicsCommandContext();

	pCommandContext->SetViewport(m_pCamera->GetViewport());

	RenderTarget* pOriginalTarget = m_pGraphics->GetDefaultRenderTarget()->GetRenderTarget();
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
		m_pRenderer->Blit(pCommandContext, pCurrentSource, pCurrentTarget, pMaterial.second);

		std::swap(pCurrentSource, pCurrentTarget);
	}

	//Do an extra blit if the shader count is odd
	if (activeMaterials % 2 == 1)
	{
		m_pRenderer->Blit(pCommandContext, m_pRenderTexture->GetRenderTarget(), pOriginalTarget);
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
	const FloatRect& viewport = m_pCamera->GetViewport();
	OnResize((int)viewport.GetWidth(), (int)viewport.GetHeight());
	GetSubsystem<InputEngine>()->OnWindowSizeChanged().AddRaw(this, &PostProcessing::OnResize);

	Renderer* pRenderer = GetSubsystem<Renderer>();
	pRenderer->AddPostProcessing(this);
}

void PostProcessing::OnResize(const int width, const int height)
{
	int msaa = m_pCamera->GetRenderTarget()->GetParentTexture()->GetMultiSample();
	m_pRenderTexture->SetSize(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::RENDERTARGET, msaa, nullptr);
	m_pRenderTexture->SetAddressMode(TextureAddressMode::CLAMP);
	m_pDepthTexture->SetSize(width, height, DXGI_FORMAT_R24G8_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, msaa, nullptr);
}

void PostProcessing::CreateUI()
{
	for (uint32 i = 0; i < m_Materials.size(); ++i)
	{
		ImGui::Checkbox(m_Materials[i].second->GetName().c_str(), &m_Materials[i].first);
	}
}