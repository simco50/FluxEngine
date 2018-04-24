#include "FluxEngine.h"
#include "D3D11GraphicsImpl.h"
#include "../DepthStencilState.h"
#include "../Graphics.h"

void* DepthStencilState::GetOrCreate(Graphics* pGraphics)
{
	unsigned int stateHash =
		(unsigned char)m_DepthEnabled << 0
		| (unsigned char)m_DepthWrite << 1
		| (unsigned char)m_DepthCompareMode << 2
		| (unsigned char)m_StencilTestEnabled << 6
		| (unsigned char)m_StencilTestMode << 7
		| (unsigned char)m_StencilTestPassOperation << 11
		| (unsigned char)m_StencilTestFailOperation << 15
		| (unsigned char)m_StencilTestZFailOperation << 19
		| (unsigned char)m_StencilCompareMask << 23
		| (unsigned char)m_StencilWriteMask << 27;

	auto state = m_DepthStencilStates.find(stateHash);
	if (state != m_DepthStencilStates.end())
		return state->second;

	AUTOPROFILE_DESC(DepthStencilState_Create, Math::ToHex(stateHash));

	m_DepthStencilStates[stateHash] = nullptr;
	void*& pState = m_DepthStencilStates[stateHash];

	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = m_DepthEnabled;
	desc.DepthWriteMask = m_DepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

	desc.DepthFunc = D3D11ComparisonFunction(m_DepthCompareMode);

	desc.StencilEnable = m_StencilTestEnabled;
	desc.StencilReadMask = m_StencilCompareMask;
	desc.StencilWriteMask = m_StencilWriteMask;

	desc.FrontFace.StencilFunc = D3D11ComparisonFunction(m_StencilTestMode);
	desc.BackFace.StencilFunc = desc.FrontFace.StencilFunc;

	desc.FrontFace.StencilPassOp = D3D11StencilOperation(m_StencilTestPassOperation);
	desc.BackFace.StencilPassOp = desc.FrontFace.StencilPassOp;

	desc.FrontFace.StencilFailOp = D3D11StencilOperation(m_StencilTestFailOperation);
	desc.BackFace.StencilFailOp = desc.FrontFace.StencilFailOp;

	desc.FrontFace.StencilDepthFailOp = D3D11StencilOperation(m_StencilTestZFailOperation);
	desc.BackFace.StencilDepthFailOp = desc.FrontFace.StencilDepthFailOp;

	HR(pGraphics->GetImpl()->GetDevice()->CreateDepthStencilState(&desc, (ID3D11DepthStencilState**)&pState));

	return pState;
}
