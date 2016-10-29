#include "stdafx.h"
#include "SpriteRenderer.h"
#include "../Graphics/Texture.h"
#include "../Scenegraph/GameObject.h"
#include "../Components/TransformComponent.h"

SpriteRenderer::SpriteRenderer(): m_pGameContext(nullptr)
{
}


SpriteRenderer::~SpriteRenderer()
{
}

void SpriteRenderer::Initialize(GameContext* pGameContext)
{
	m_pGameContext = pGameContext;

	CreateInputLayout();
	LoadShaderVariables();
	CreateBuffer();
}

void SpriteRenderer::Update()
{
	if(m_SpriteCapacity < (int)m_Vertices.size())
	{
		m_SpriteCapacity = (int)m_Vertices.size();
		CreateBuffer();
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	m_pGameContext->Engine->D3DeviceContext->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, m_Vertices.data(), m_Vertices.size() * sizeof(SpriteVertex));
	m_pGameContext->Engine->D3DeviceContext->Unmap(m_pVertexBuffer.Get(), 0);
}

void SpriteRenderer::Render()
{
	m_pGameContext->Engine->D3DeviceContext->IASetInputLayout(m_pInputLayout.Get());
	UINT stride = 0;
	UINT offset = 0;
	m_pGameContext->Engine->D3DeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pGameContext->Engine->D3DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);

	m_pTextureVar->SetResource(ResourceManager::Load<Texture>(L"./Resources/Textures/Patrick.tga")->GetResourceView());
	for (size_t i = 0; i < techDesc.Passes; i++)
	{
		m_pTechnique->GetPassByIndex(i)->Apply(0, m_pGameContext->Engine->D3DeviceContext);
		m_pGameContext->Engine->D3DeviceContext->Draw(1, 0);
	}
}

void SpriteRenderer::LoadShaderVariables()
{
	BIND_AND_CHECK_NAME(m_pTextureVar, gTexture, AsShaderResource);
}

void SpriteRenderer::CreateInputLayout()
{
	m_pEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/SpriteRenderer.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	D3D11_INPUT_ELEMENT_DESC desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(desc) / sizeof(desc[0]);

	D3DX11_PASS_DESC passDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pGameContext->Engine->D3Device->CreateInputLayout(desc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pInputLayout.GetAddressOf()))
}

void SpriteRenderer::CreateBuffer()
{
	m_pVertexBuffer.Reset();

	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(SpriteVertex) * m_SpriteCapacity;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	HR(m_pGameContext->Engine->D3Device->CreateBuffer(&bd, nullptr, m_pVertexBuffer.GetAddressOf()))
}