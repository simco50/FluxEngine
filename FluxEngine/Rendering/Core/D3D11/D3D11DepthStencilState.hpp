#include "D3D11GraphicsImpl.h"
void* DepthStencilState::GetOrCreate(Graphics* pGraphics)
{
	unsigned int stateHash =
		(unsigned char)m_DepthEnabled << 0
		| (unsigned char)m_DepthCompareMode << 1
		| (unsigned char)m_StencilTestEnabled << 5
		| (unsigned char)m_StencilTestMode << 6
		| (unsigned char)m_StencilTestPassOperation << 10
		| (unsigned char)m_StencilTestFailOperation << 14
		| (unsigned char)m_StencilTestZFailOperation << 18
		| (unsigned char)m_StencilCompareMask << 22
		| (unsigned char)m_StencilWriteMask << 26;

	auto state = m_DepthStencilStates.find(stateHash);
	if (state != m_DepthStencilStates.end())
		return state->second;

	AUTOPROFILE_DESC(DepthStencilState_Create, ToHex(stateHash));

	m_DepthStencilStates[stateHash] = nullptr;
	void*& pState = m_DepthStencilStates[stateHash];

	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = m_DepthEnabled;
	desc.DepthWriteMask = m_DepthEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	switch (m_DepthCompareMode)
	{
	case CompareMode::ALWAYS:
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		break;
	case CompareMode::EQUAL:
		desc.DepthFunc = D3D11_COMPARISON_EQUAL;
		break;
	case CompareMode::NOTEQUAL:
		desc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;
		break;
	case CompareMode::LESS:
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		break;
	case CompareMode::LESSEQUAL:
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		break;
	case CompareMode::GREATER:
		desc.DepthFunc = D3D11_COMPARISON_GREATER;
		break;
	case CompareMode::GREATEREQUAL:
		desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		break;
	}

	desc.StencilEnable = m_StencilTestEnabled;
	desc.StencilReadMask = m_StencilCompareMask;
	desc.StencilWriteMask = m_StencilWriteMask;

	switch (m_StencilTestMode)
	{
	case CompareMode::ALWAYS:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		break;
	case CompareMode::EQUAL:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		break;
	case CompareMode::NOTEQUAL:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		break;
	case CompareMode::LESS:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_LESS;
		break;
	case CompareMode::LESSEQUAL:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;
		break;
	case CompareMode::GREATER:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_GREATER;
		break;
	case CompareMode::GREATEREQUAL:
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;
		break;
	}
	switch (m_StencilTestPassOperation)
	{
	case StencilOperation::KEEP:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		break;
	case StencilOperation::ZERO:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
		break;
	case StencilOperation::REF:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::INCR:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
		break;
	case StencilOperation::DECR:
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_DECR;
		break;
	}

	switch (m_StencilTestFailOperation)
	{
	case StencilOperation::KEEP:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		break;
	case StencilOperation::ZERO:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		break;
	case StencilOperation::REF:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::INCR:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INCR;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_INCR;
		break;
	case StencilOperation::DECR:
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_DECR;
		break;
	}

	switch (m_StencilTestZFailOperation)
	{
	case StencilOperation::KEEP:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		break;
	case StencilOperation::ZERO:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		break;
	case StencilOperation::REF:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::INCR:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		break;
	case StencilOperation::DECR:
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		break;
	}

	HR(pGraphics->GetImpl()->GetDevice()->CreateDepthStencilState(&desc, (ID3D11DepthStencilState**)&pState));

	return pState;
}