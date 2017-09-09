#include "stdafx.h"
#include "DeferredRenderer.h"
#include "GBufferMaterial.h"
#include "../Graphics/RenderTarget.h"
#include "../Components/Camera.h"
#include "../Components/Transform.h"
#include "ShadowMapper.h"
#include "QuadRenderer.h"

DeferredRenderer::DeferredRenderer()
{
}


DeferredRenderer::~DeferredRenderer()
{
}

void DeferredRenderer::Initialize(GameContext* pGameContext)
{
	m_pGameContext = pGameContext;

	m_pMaterial = make_unique<GBufferMaterial>();
	m_pMaterial->Initialize(pGameContext);

	m_pQuadRenderer = new QuadRenderer(pGameContext);
}

void DeferredRenderer::CreateGBuffer()
{
	m_pGBuffer.clear();

	int width = m_pGameContext->Engine->GameSettings.Width;
	int height = m_pGameContext->Engine->GameSettings.Height;
	m_pGBuffer.resize(GBUFFER_SIZE);

	RENDER_TARGET_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ColorSRV = true;
	desc.DepthBuffer = false;
	m_pGBuffer[DIFFUSE] = make_unique<RenderTarget>(m_pGameContext->Engine->D3Device, m_pGameContext->Engine->D3DeviceContext);
	m_pGBuffer[DIFFUSE]->Create(desc);

	desc.ColorFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	m_pGBuffer[NORMAL] = make_unique<RenderTarget>(m_pGameContext->Engine->D3Device, m_pGameContext->Engine->D3DeviceContext);
	m_pGBuffer[NORMAL]->Create(desc);
}

void DeferredRenderer::Begin()
{
	ClearRenderTargets();
	SetRenderTargets();
}

void DeferredRenderer::End()
{
	//Set render target to null
	ID3D11RenderTargetView* pRTV[GBUFFER_SIZE] = { nullptr };
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(GBUFFER_SIZE, pRTV, nullptr);

	//Set to the default render target
	ID3D11RenderTargetView* pDefaultRTV = m_pGameContext->Engine->DefaultRenderTarget->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(1, &pDefaultRTV, nullptr);

	//Set the gbuffer's material variables
	m_pMaterial->SetDiffuseSRV(m_pGBuffer[DIFFUSE]->GetColorSRV());
	m_pMaterial->SetNormalSRV(m_pGBuffer[NORMAL]->GetColorSRV());
	m_pMaterial->SetDepthSRV(m_pGameContext->Engine->DefaultRenderTarget->GetDepthSRV());
	m_pMaterial->SetEyePos(m_pGameContext->Scene->Camera->GetTransform()->GetWorldPosition());
	m_pMaterial->SetViewProjInv(m_pGameContext->Scene->Camera->GetViewProjectionInverse());

	//Shadow mapping
	if (m_pGameContext->Scene->ShadowMapper != nullptr)
	{
		m_pMaterial->SetShadowMap(m_pGameContext->Scene->ShadowMapper->GetShadowMap());
		m_pMaterial->SetLightVP(m_pGameContext->Scene->ShadowMapper->GetLightVP());
		m_pMaterial->SetLightDirection(m_pGameContext->Scene->ShadowMapper->GetLightDirection());
	}
	//Draw a quad on the screen using the gbuffer material
	m_pQuadRenderer->Render(m_pMaterial.get());

	//Unbind GBUFFER and DSV from input
	ID3D11ShaderResourceView* pSRV[GBUFFER_SIZE + 2] = { nullptr };
	m_pGameContext->Engine->D3DeviceContext->PSSetShaderResources(0, GBUFFER_SIZE + 2, pSRV);
}

void DeferredRenderer::ClearRenderTargets()
{
	XMFLOAT4 clearColor(0.0f, 0.0f, 0.0f, 1.0f);
	for (size_t i = 0; i < GBUFFER_SIZE; i++)
	{
		ID3D11RenderTargetView* pRTV = m_pGBuffer[i]->GetRenderTargetView();
		m_pGameContext->Engine->D3DeviceContext->ClearRenderTargetView(pRTV, reinterpret_cast<float*>(&clearColor));
	}
	m_pGameContext->Engine->D3DeviceContext->ClearDepthStencilView(m_pGameContext->Engine->DefaultRenderTarget->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3D11RenderTargetView* pRTV = m_pGBuffer[0]->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->ClearRenderTargetView(pRTV, reinterpret_cast<float*>(&m_pGameContext->Engine->GameSettings.ClearColor));
}

void DeferredRenderer::SetRenderTargets()
{
	vector<ID3D11RenderTargetView*> pRTVs(GBUFFER_SIZE);
	for (size_t i = 0; i < GBUFFER_SIZE; i++)
		pRTVs[i] = m_pGBuffer[i]->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(GBUFFER_SIZE, pRTVs.data(), m_pGameContext->Engine->DefaultRenderTarget->GetDepthStencilView());
}
