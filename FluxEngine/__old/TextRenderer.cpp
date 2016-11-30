#include "stdafx.h"
#include "TextRenderer.h"

#include "../Content/ResourceManager.h"
#include "SpriteFont.h"
#include "../Graphics/Texture.h"


TextRenderer::TextRenderer() :
				m_BufferSize(500),
				m_InputLayoutSize(0),
				m_NumCharacters(0),
				m_pEffect(nullptr),
				m_pTechnique(nullptr),
				m_pTransfromMatrixV(nullptr),
				m_pTextureSizeV(nullptr), 
				m_pTextureSRV(nullptr),
				m_pInputLayout(nullptr),
				m_pVertexBuffer(nullptr),
				m_SpriteFonts(vector<SpriteFont*>())
{
}

TextRenderer::~TextRenderer()
{
	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
}

void TextRenderer::InitRenderer(EngineContext* pEngineContext)
{
	m_pEngineContext = pEngineContext;

	//Effect
	m_pEffect = ResourceManager::Load<ID3DX11Effect>(L"./Resources/Shaders/TextRenderer.fx");
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

	//CREATE INPUT LAYOUT
	//EffectHelper::BuildInputLayout(pDevice, m_pTechnique, &m_pInputLayout, m_InputLayoutSize);

	D3D11_INPUT_ELEMENT_DESC elementDesc[] =
	{
		{ "TEXCOORD", 2, DXGI_FORMAT_R32_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	D3DX11_PASS_DESC passDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(m_pEngineContext->D3Device->CreateInputLayout(elementDesc, 5, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout))

	//Transform Matrix
	MakeTransform();
}

void TextRenderer::MakeTransform()
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

void TextRenderer::DrawText(SpriteFont* pFont, const wstring& text, Vector2 position, Vector4 color)
{
	m_NumCharacters += wcslen(text.c_str());
	pFont->m_TextCache.push_back(TextCache(text, position, color));
	if (!pFont->m_IsAddedToRenderer)
	{
		m_SpriteFonts.push_back(pFont);
		pFont->m_IsAddedToRenderer = true;
	}
}

void TextRenderer::Draw()
{
	if (m_SpriteFonts.size() <= 0)
		return;

	UpdateBuffer();

	//Set Render Pipeline
	m_pEngineContext->D3DeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(TextVertex);
	UINT offset = 0;
	m_pEngineContext->D3DeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_pEngineContext->D3DeviceContext->IASetInputLayout(m_pInputLayout);

	for each(SpriteFont* pFont in m_SpriteFonts)
	{
		//Set Texture
		m_pTextureSRV->SetResource(pFont->m_pTexture->GetResourceView());

		//Set Texture Size
		float size[2] = { (float)pFont->m_pTexture->GetWidth(), (float)pFont->m_pTexture->GetHeight() };
		m_pTextureSizeV->SetFloatVector(size);

		//Set Transform
		m_pTransfromMatrixV->SetMatrix(reinterpret_cast<float*>(&m_Transform));

		D3DX11_TECHNIQUE_DESC techDesc;
		m_pTechnique->GetDesc(&techDesc);
		for (UINT i = 0; i < techDesc.Passes; ++i)
		{
			m_pTechnique->GetPassByIndex(i)->Apply(0, m_pEngineContext->D3DeviceContext);
			m_pEngineContext->D3DeviceContext->Draw(pFont->m_BufferSize, pFont->m_BufferStart);
		}

		pFont->m_IsAddedToRenderer = false;
	}

	m_SpriteFonts.clear();
}

void TextRenderer::UpdateBuffer()
{
	if (!m_pVertexBuffer || m_NumCharacters > m_BufferSize)
	{
		//Release the buffer if it exists
		SafeRelease(m_pVertexBuffer);

		//Set buffersize if needed
		if (m_NumCharacters > m_BufferSize)
			m_BufferSize = m_NumCharacters;

		//Create a new buffer
		D3D11_BUFFER_DESC buffDesc;
		buffDesc.Usage = D3D11_USAGE_DYNAMIC;
		buffDesc.ByteWidth = sizeof(TextVertex) * m_BufferSize;
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;

		HR(m_pEngineContext->D3Device->CreateBuffer(&buffDesc, nullptr, &m_pVertexBuffer))
	}

	//Fill Buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_pEngineContext->D3DeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
	int bufferPosition = 0;
	for each (SpriteFont* pFont in m_SpriteFonts)
	{
		pFont->m_BufferStart = bufferPosition;
		for each(TextCache cache in pFont->m_TextCache)
		{			
			CreateTextVertices(pFont, cache, (TextVertex*) mappedResource.pData, bufferPosition);
		}

		pFont->m_BufferSize = bufferPosition - pFont->m_BufferStart;
		pFont->m_TextCache.clear(); //Clear the cache
	}
	m_pEngineContext->D3DeviceContext->Unmap(m_pVertexBuffer, 0);

	m_NumCharacters = 0;
}

void TextRenderer::CreateTextVertices(SpriteFont* pFont, const TextCache& info, TextVertex* pBuffer, int& bufferPosition)
{
	int totalAdvanceX = 0;
	for (wchar_t charId : info.Text)
	{
		if (SpriteFont::IsCharValid(charId) && pFont->GetMetric(charId).IsValid)
		{
			auto metric = pFont->GetMetric(charId);

			if (charId == ' ')
			{
				totalAdvanceX += metric.AdvanceX;
				continue;
			}

			TextVertex vertexText;
			vertexText.Position.x = info.Position.x + totalAdvanceX + metric.OffsetX;
			vertexText.Position.y = info.Position.y + metric.OffsetY;
			vertexText.Position.z = 0.9f;
			vertexText.Color = info.Color;
			vertexText.TexCoord = metric.TexCoord;
			vertexText.CharacterDimension = Vector2(metric.Width, metric.Height);
			vertexText.ChannelId = metric.Channel;

			pBuffer[bufferPosition] = vertexText;

			++bufferPosition;
			totalAdvanceX += metric.AdvanceX;

		}
		else
		{
			DebugLog::LogFormat(LogType::WARNING, L"TextRenderer::CreateTextVertices > Char not supported for current font.\nCHARACTER: %c (%i)\nFONT: %s", charId,(int)charId, pFont->m_FontName.c_str());
		}
	}
}
