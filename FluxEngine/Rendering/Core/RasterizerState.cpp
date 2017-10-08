#include "stdafx.h"
#include "RasterizerState.h"
#include "Graphics.h"

RasterizerState::RasterizerState()
{

}

ID3D11RasterizerState* RasterizerState::Create(ID3D11Device* pDevice)
{
	m_pRasterizerState.Reset();

	D3D11_RASTERIZER_DESC desc = {};
	desc.AntialiasedLineEnable = false;
	switch (m_CullMode)
	{
	case CullMode::FRONT:
		desc.CullMode = D3D11_CULL_FRONT;
		break;
	case CullMode::BACK:
		desc.CullMode = D3D11_CULL_BACK;
		break;
	case CullMode::NONE:
		desc.CullMode = D3D11_CULL_NONE;
		break;
	}

	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.DepthClipEnable = true;
	switch (m_FillMode)
	{
	case FillMode::SOLID:
		desc.FillMode = D3D11_FILL_SOLID;
		break;
	case FillMode::WIREFRAME:
		desc.FillMode = D3D11_FILL_WIREFRAME;
		break;
	}
	desc.FrontCounterClockwise = false;
	desc.MultisampleEnable = m_MultisampleEnabled;
	desc.ScissorEnable = m_ScissorEnabled;
	desc.SlopeScaledDepthBias = 0.0f;

	HR(pDevice->CreateRasterizerState(&desc, m_pRasterizerState.GetAddressOf()));

	return m_pRasterizerState.Get();
}

void RasterizerState::SetFillMode(const FillMode fillMode)
{
	if (fillMode != m_FillMode)
	{
		m_FillMode = fillMode;
		m_IsDirty = true;
	}
}

void RasterizerState::SetCullMode(const CullMode cullMode)
{
	if (cullMode != m_CullMode)
	{
		m_CullMode = cullMode;
		m_IsDirty = true;
	}
}

void RasterizerState::SetScissorEnabled(const bool enabled)
{
	if (enabled != m_ScissorEnabled)
	{
		m_ScissorEnabled = enabled;
		m_IsDirty = true;
	}
}

void RasterizerState::SetMultisampleEnabled(const bool enabled)
{
	if (enabled != m_MultisampleEnabled)
	{
		m_MultisampleEnabled = enabled;
		m_IsDirty = true;
	}
}