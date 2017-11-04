#include "D3D11GraphicsImpl.h"
void* RasterizerState::GetOrCreate(Graphics* pGraphics)
{
	unsigned int stateHash =
		(unsigned char)m_ScissorEnabled << 0
		| (unsigned char)m_MultisampleEnabled << 1
		| (unsigned char)m_FillMode << 2
		| (unsigned char)m_CullMode << 6
		| (unsigned char)m_LineAntiAlias << 10;

	auto state = m_RasterizerStates.find(stateHash);
	if (state != m_RasterizerStates.end())
		return state->second;

	AUTOPROFILE(CreateRasterizerState);

	m_RasterizerStates[stateHash] = nullptr;
	void*& pState = m_RasterizerStates[stateHash];

	D3D11_RASTERIZER_DESC desc = {};
	desc.AntialiasedLineEnable = m_LineAntiAlias;
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

	HR(pGraphics->GetImpl()->GetDevice()->CreateRasterizerState(&desc, (ID3D11RasterizerState**)&pState));

	return pState;
}
