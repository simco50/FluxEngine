#include "stdafx.h"
#include "QuadRenderer.h"
#include "../Materials/Material.h"
#include "../Materials/DefaultQuadMaterial.h"

QuadRenderer::QuadRenderer(GameContext* pGameContext):
m_pGameContext(pGameContext)
{
	CreateVertexBuffer();
	m_pDefaultMaterial = new DefaultQuadMaterial();
	m_pDefaultMaterial->Initialize(pGameContext);
}


QuadRenderer::~QuadRenderer()
{
}

void QuadRenderer::Render(Material* pMaterial)
{
	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(pMaterial->GetInputLayout());

	UINT strides = sizeof(VertexPosTex);
	UINT offset = 0;
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offset);

	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	D3DX11_TECHNIQUE_DESC techDesc;
	pMaterial->GetTechnique()->GetDesc(&techDesc);
	for (size_t i = 0; i < techDesc.Passes; i++)
	{
		pMaterial->GetTechnique()->GetPassByIndex(0)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->Draw(4, 0);
	}
}

void QuadRenderer::Render(ID3D11ShaderResourceView* pSRV)
{
	m_pDefaultMaterial->SetTexture(pSRV);
	Render(m_pDefaultMaterial);
}

void QuadRenderer::CreateVertexBuffer()
{
	vector<VertexPosTex> vertices(4);
	vertices[0] = VertexPosTex(XMFLOAT3(-1, 1, 0), XMFLOAT2(0, 0));
	vertices[1] = VertexPosTex(XMFLOAT3(1, 1, 0), XMFLOAT2(1, 0));
	vertices[2] = VertexPosTex(XMFLOAT3(-1, -1, 0), XMFLOAT2(0, 1));
	vertices[3] = VertexPosTex(XMFLOAT3(1, -1, 0), XMFLOAT2(1, 1));

	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(VertexPosTex) * vertices.size();
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices.data();

	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, &initData, m_pVertexBuffer.GetAddressOf()));
}
