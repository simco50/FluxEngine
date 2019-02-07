#include "FluxEngine.h"
#include "../RasterizerState.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"
#include "D3D11Helpers.h"

void* RasterizerState::GetOrCreate(Graphics* pGraphics)
{
	m_IsDirty = false;

	unsigned int stateHash =
		(unsigned char)m_ScissorEnabled << 0
		| (unsigned char)m_MultisampleEnabled << 1
		| (unsigned char)m_FillMode << 2
		| (unsigned char)m_CullMode << 6
		| (unsigned char)m_LineAntiAlias << 10;

	auto state = m_RasterizerStates.find(stateHash);
	if (state != m_RasterizerStates.end())
		return state->second;

	AUTOPROFILE_DESC(RasterizerState_Create, Math::ToHex(stateHash));

	m_RasterizerStates[stateHash] = nullptr;
	void*& pState = m_RasterizerStates[stateHash];

	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
	desc.AntialiasedLineEnable = m_LineAntiAlias;
	desc.CullMode = D3D11CullMode(m_CullMode);
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.DepthClipEnable = true;
	desc.FillMode = D3D11FillMode(m_FillMode);
	desc.FrontCounterClockwise = false;
	desc.MultisampleEnable = m_MultisampleEnabled;
	desc.ScissorEnable = m_ScissorEnabled;
	desc.SlopeScaledDepthBias = 0.0f;

	HR(pGraphics->GetImpl()->GetDevice()->CreateRasterizerState(&desc, (ID3D11RasterizerState**)&pState));

	return pState;
}
