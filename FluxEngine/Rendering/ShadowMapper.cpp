#include "stdafx.h"
#include "ShadowMapper.h"
#include "../Graphics/RenderTarget.h"
#include "../Rendering/ShadowMapMaterial.h"
#include "../Components/MeshRenderer.h"
#include "../Scenegraph/GameObject.h"
#include "../Components/Camera.h"

ShadowMapper::ShadowMapper()
{
}


ShadowMapper::~ShadowMapper()
{
}

void ShadowMapper::Initialize(GameContext* pGameContext)
{
	m_pGameContext = pGameContext;

	SetResolution(m_ShadowMapWidth, m_ShadowMapHeight);

	m_pShadowMapMaterial = make_unique<ShadowMapMaterial>();
	m_pShadowMapMaterial->Initialize(m_pGameContext);
}

void ShadowMapper::Begin()
{
	m_pGameContext->Engine->D3DeviceContext->ClearDepthStencilView(m_pRenderTarget->GetDepthStencilView(), D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);

	ID3D11RenderTargetView* pRTV = m_pRenderTarget->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(1, &pRTV, m_pRenderTarget->GetDepthStencilView());

	m_pGameContext->Engine->D3DeviceContext->RSSetViewports(1, &m_Viewport);

	m_pShadowMapMaterial->SetLightVP(m_ViewProjectionMatrix);
}

void ShadowMapper::Render(GameObject* pGameObject)
{
	MeshRenderer* pMesh = pGameObject->GetComponent<MeshRenderer>();
	if (pMesh == nullptr)
		return;
	Material* pObjectMaterial = pMesh->GetMaterial();
	pMesh->SetMaterial(m_pShadowMapMaterial.get());

	pMesh->Render();
	for (size_t i = 0; i < pGameObject->m_pChildren.size(); i++)
		Render(pGameObject->m_pChildren[i]);

	pMesh->SetMaterial(pObjectMaterial);
}

void ShadowMapper::End()
{
	m_pGameContext->Engine->D3DeviceContext->RSSetViewports(1, &m_pGameContext->Scene->Camera->GetViewport());
	ID3D11RenderTargetView* pRTV = m_pGameContext->Engine->DefaultRenderTarget->GetRenderTargetView();
	m_pGameContext->Engine->D3DeviceContext->OMSetRenderTargets(1, &pRTV, m_pGameContext->Engine->DefaultRenderTarget->GetDepthStencilView());
}

ID3D11ShaderResourceView* ShadowMapper::GetShadowMap() const
{
	return m_pRenderTarget->GetDepthSRV();
}

void ShadowMapper::SetResolution(const int width, const int height)
{
	m_ShadowMapWidth = width;
	m_ShadowMapHeight = height;

	m_Viewport.Width = (float)m_ShadowMapWidth;
	m_Viewport.Height = (float)m_ShadowMapHeight;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;

	m_pRenderTarget.reset();

	RENDER_TARGET_DESC desc;
	desc.Width = m_ShadowMapWidth;
	desc.Height = m_ShadowMapHeight;
	desc.ColorBuffer = true;
	desc.ColorSRV = false;
	desc.MSAA = false;
	desc.DepthBuffer = true;
	desc.DepthSRV = true;
	desc.DepthFormat = DXGI_FORMAT_D32_FLOAT;
	m_pRenderTarget = make_unique<RenderTarget>(m_pGameContext->Engine);
	m_pRenderTarget->Create(&desc);
}

void ShadowMapper::SetLight(const XMFLOAT3& position, const XMFLOAT3& direction)
{
	m_LightDirection = direction;

	XMVECTOR xmPos = XMLoadFloat3(&position);
	XMVECTOR xmDir = XMLoadFloat3(&direction);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMVECTOR xmUp = XMLoadFloat3(&up);
	XMMATRIX view = XMMatrixLookAtLH(xmPos, xmDir + xmPos, xmUp);

	float viewWidth = (float)m_ShadowMapWidth / m_ShadowMapHeight * m_Size;
	float viewHeight = (float)m_Size;

	XMMATRIX projection = XMMatrixOrthographicLH(viewWidth, viewHeight, 0.1f, 500.0f);
	XMStoreFloat4x4(&m_ViewProjectionMatrix, view * projection);
}