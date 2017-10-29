#include "stdafx.h"
#include "BlendState.h"
#include "Graphics.h"

BlendState::BlendState()
{

}

BlendState::~BlendState()
{
	for (auto& pState : m_BlendStates)
	{
		SafeRelease(pState.second);
	}
	m_BlendStates.clear();
}

void BlendState::SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage)
{
	if (blendMode != m_BlendMode || alphaToCoverage != m_AlphaToCoverage)
	{
		m_BlendMode = blendMode;
		m_AlphaToCoverage = alphaToCoverage;
		m_IsDirty = true;
	}
}

void BlendState::SetColorWrite(const ColorWrite colorWriteMask /*= ColorWrite::ALL*/)
{
	if (m_ColorWriteMask != colorWriteMask)
	{
		m_ColorWriteMask = colorWriteMask;
		m_IsDirty = true;
	}
}

void* BlendState::GetOrCreate(Graphics* pGraphics)
{
	unsigned int stateHash =
		 (unsigned char)m_BlendMode << 0
		| (unsigned char)m_AlphaToCoverage << 8
		| (unsigned char)m_ColorWriteMask << 16;

	auto state = m_BlendStates.find(stateHash);
	if (state != m_BlendStates.end())
		return state->second;

	AUTOPROFILE(CreateBlendState);

	m_BlendStates[stateHash] = nullptr;
	void*& pBlendState = m_BlendStates[stateHash];

	D3D11_BLEND_DESC desc = {};
	desc.AlphaToCoverageEnable = m_AlphaToCoverage;
	desc.IndependentBlendEnable = false;
	desc.RenderTarget[0].BlendEnable = m_BlendMode == BlendMode::REPLACE ? false : true;
	desc.RenderTarget[0].RenderTargetWriteMask = (unsigned int)m_ColorWriteMask;

	switch (m_BlendMode)
	{
	case BlendMode::REPLACE:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ADD:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::MULTIPLY:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_COLOR;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::ADDALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::PREMULALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::INVDESTALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		break;
	case BlendMode::SUBTRACT:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT;
		break;
	case BlendMode::SUBTRACTALPHA:
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT;
		break;
	}

	HR(pGraphics->GetDevice()->CreateBlendState(&desc, (ID3D11BlendState**)&pBlendState));

	return pBlendState;
}