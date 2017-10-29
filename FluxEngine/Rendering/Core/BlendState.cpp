#include "stdafx.h"
#include "BlendState.h"
#include "Graphics.h"

#ifdef D3D11
#include "D3D11/D3D11BlendState.hpp"
#endif

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