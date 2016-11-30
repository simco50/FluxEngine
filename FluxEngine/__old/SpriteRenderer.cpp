//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "SpriteRenderer.h"
#include "../Content/ResourceManager.h"
#include "../Graphics/Texture.h"
#include "../Core/FluxCore.h"


SpriteRenderer::SpriteRenderer() :
	m_Sprites(vector<SpriteVertex>()),
	m_Textures(vector<Texture*>()),
	m_BufferSize(50),
	m_InputLayoutSize(0),
	m_pEffect(nullptr),
	m_pTechnique(nullptr),
	m_pInputLayout(nullptr),
	m_pVertexBuffer(nullptr),
	m_pImmediateVertexBuffer(nullptr),
	m_pTransfromMatrixV(nullptr),
	m_pTextureSizeV(nullptr),
	m_pTextureSRV(nullptr),
	m_pEngineContext(nullptr)
{
}


SpriteRenderer::~SpriteRenderer()
{
	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pImmediateVertexBuffer);

	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::InitRenderer(EngineContext* pEngineContext)
{
	m_pEngineContext = pEngineContext;

	//Effect
	m_pEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/SpriteRenderer.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	m_pTransfromMatrixV = m_pEffect->GetVariableByName("gTransform")->AsMatrix();
	if (!m_pTransfromMatrixV->IsValid())
	{
		DebugLog::Log(L"SpriteRenderer::Initialize() > Shader variable \'gTransform\' not valid!", LogType::ERROR);
		return;
	}

	m_pTextureSizeV = m_pEffect->GetVariableByName("gTextureSize")->AsVector();
	if (!m_pTextureSizeV->IsValid())
	{
		DebugLog::Log(L"SpriteRenderer::Initialize() > Shader variable \'gTextureSize\' not valid!", LogType::ERROR);
		return;
	}

	m_pTextureSRV = m_pEffect->GetVariableByName("gSpriteTexture")->AsShaderResource();
	if (!m_pTextureSRV->IsValid())
	{
		DebugLog::Log(L"SpriteRenderer::Initialize() > Shader variable \'gSpriteTexture\' not valid!", LogType::ERROR);
		return;
	}

	D3D11_INPUT_ELEMENT_DESC elementDesc[] =
	{
		{ "TEXCOORD", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	D3DX11_PASS_DESC passDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pEngineContext->D3Device->CreateInputLayout(elementDesc, 4, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout))

	MakeTransform();
}

void SpriteRenderer::MakeTransform()
{
	//Transform Matrix
	auto settings = m_pEngineContext->GameSettings;
	float scaleX = (settings.Width>0) ? 2.0f / settings.Width : 0;
	float scaleY = (settings.Height>0) ? 2.0f / settings.Height : 0;

	m_Transform._11 = scaleX; m_Transform._12 = 0; m_Transform._13 = 0; m_Transform._14 = 0;
	m_Transform._21 = 0; m_Transform._22 = -scaleY; m_Transform._23 = 0; m_Transform._24 = 0;
	m_Transform._31 = 0; m_Transform._32 = 0; m_Transform._33 = 1; m_Transform._34 = 0;
	m_Transform._41 = -1; m_Transform._42 = 1; m_Transform._43 = 0; m_Transform._44 = 1;
}

void SpriteRenderer::UpdateBuffer()
{
	// if the vertex buffer does not exists, or the number of sprites is bigger then the buffer size
	//		release the buffer
	//		update the buffer size (if needed)
	//		Create a new buffer. Make sure the Usage flag is set to Dynamic, you bind to the vertex buffer
	//		and you set the cpu access flags to access_write
	//
	//		Finally create the buffer. You can use the device in the game context. Be sure to log the HResult!
	if(m_Sprites.size() > m_BufferSize || m_pVertexBuffer == nullptr)
	{
		SafeRelease(m_pVertexBuffer);

		if(m_pVertexBuffer != nullptr) 
			m_pVertexBuffer->Release();

		m_BufferSize = m_Sprites.size();

		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.ByteWidth = sizeof(SpriteVertex) * m_BufferSize;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;

		HR(m_pEngineContext->D3Device->CreateBuffer(&bd, nullptr, &m_pVertexBuffer))
	}
	// Finally fill the buffer. You will need to create a D3D11_MAPPED_SUBRESOURCE
	D3D11_MAPPED_SUBRESOURCE data;
	// Next you will need to use the device context to map the vertex buffer to the mapped resource
	m_pEngineContext->D3DeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &data);
	// use memcpy to copy all our sprites to the mapped resource
	memcpy(data.pData, m_Sprites.data(), sizeof(SpriteVertex) * m_Sprites.size());
	// unmap the vertex buffer
	m_pEngineContext->D3DeviceContext->Unmap(m_pVertexBuffer, 0);
}

void SpriteRenderer::Draw()
{
	if (m_Sprites.size() <= 0)
		return;

	UpdateBuffer();

	//Set Render Pipeline
	m_pEngineContext->D3DeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(SpriteVertex);
	UINT offset = 0;
	m_pEngineContext->D3DeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_pEngineContext->D3DeviceContext->IASetInputLayout(m_pInputLayout);

	UINT batchSize = 1;
	UINT batchOffset = 0;
	UINT spriteCount = m_Sprites.size();
	for (UINT i = 0; i < spriteCount; ++i)
	{
		if (i < (spriteCount - 1) && m_Sprites[i].TextureId == m_Sprites[i + 1].TextureId)
		{
			++batchSize;
			continue;
		}

		//Set Texture
		auto texData = m_Textures[m_Sprites[i].TextureId];
		m_pTextureSRV->SetResource(texData->GetResourceView());

		//Set Texture Size
		float size[2] = { (float)texData->GetWidth(), (float)texData->GetHeight() };
		m_pTextureSizeV->SetFloatVector(size);

		//Set Transform
		m_pTransfromMatrixV->SetMatrix(reinterpret_cast<float*>(&m_Transform));

		D3DX11_TECHNIQUE_DESC techDesc;
		m_pTechnique->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_pTechnique->GetPassByIndex(p)->Apply(0, m_pEngineContext->D3DeviceContext);
			m_pEngineContext->D3DeviceContext->Draw(batchSize, batchOffset);
		}

		batchOffset += batchSize;
		batchSize = 1;
	}

	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::DrawImmediate(ID3D11ShaderResourceView* pSrv, Vector2 position, Vector4 color, Vector2 pivot, Vector2 scale, float rotation)
{
	//Create Immediate VB
	if (!m_pImmediateVertexBuffer)
	{
		//Create a new buffer
		D3D11_BUFFER_DESC buffDesc;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		buffDesc.ByteWidth = sizeof(SpriteVertex);
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;

		auto hr = m_pEngineContext->D3Device->CreateBuffer(&buffDesc, nullptr, &m_pImmediateVertexBuffer);
		if (DebugLog::LogHRESULT(L"SpriteRenderer::DrawImmediate > Immediate Vertexbuffer creation failed!", hr))
			return;
	}

	//Map Vertex
	SpriteVertex vertex;
	vertex.TextureId = 0;
	vertex.TransformData = Vector4(position.x, position.y, 0, rotation);
	vertex.TransformData2 = Vector4(pivot.x, pivot.y, scale.x, scale.y);
	vertex.Color = color;

	if (!m_ImmediateVertex.Equals(vertex))
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		m_pEngineContext->D3DeviceContext->Map(m_pImmediateVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &vertex, sizeof(SpriteVertex));
		m_pEngineContext->D3DeviceContext->Unmap(m_pImmediateVertexBuffer, 0);
		m_ImmediateVertex = vertex;
	}

	//Set Render Pipeline
	m_pEngineContext->D3DeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(SpriteVertex);
	UINT offset = 0;
	m_pEngineContext->D3DeviceContext->IASetVertexBuffers(0, 1, &m_pImmediateVertexBuffer, &stride, &offset);
	m_pEngineContext->D3DeviceContext->IASetInputLayout(m_pInputLayout);

	//Set Texture
	m_pTextureSRV->SetResource(pSrv);

	ID3D11Resource* pResource;
	pSrv->GetResource(&pResource);
	D3D11_TEXTURE2D_DESC texDesc;
	auto texResource = reinterpret_cast<ID3D11Texture2D*>(pResource);
	texResource->GetDesc(&texDesc);
	texResource->Release();

	//Set Texture Size
	auto texSize = Vector2(static_cast<float>(texDesc.Width), static_cast<float>(texDesc.Height));
	m_pTextureSizeV->SetFloatVector(reinterpret_cast<float*>(&texSize));

	//Set Transform
	m_pTransfromMatrixV->SetMatrix(reinterpret_cast<float*>(&m_Transform));

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		m_pTechnique->GetPassByIndex(i)->Apply(0, m_pEngineContext->D3DeviceContext);
		m_pEngineContext->D3DeviceContext->Draw(1, 0);
	}
}

void SpriteRenderer::Draw(Texture* pTexture, Vector2 position, Vector4 color, Vector2 pivot, Vector2 scale, float rotation, float depth)
{

	SpriteVertex vertex;

	auto it = find(m_Textures.begin(), m_Textures.end(), pTexture);
	
	if (it == m_Textures.end())
	{
		m_Textures.push_back(pTexture);
		vertex.TextureId = m_Textures.size() - 1;
	}
	else
	{
		vertex.TextureId = it - m_Textures.begin();
	}

	vertex.TransformData = Vector4(position.x, position.y, depth, rotation);
	vertex.TransformData2 = Vector4(pivot.x, pivot.y, scale.x, scale.y);
	vertex.Color = color;

	m_Sprites.push_back(vertex);
}


