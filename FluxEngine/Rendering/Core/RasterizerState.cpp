#include "FluxEngine.h"
#include "RasterizerState.h"
#include "Graphics.h"

RasterizerState::RasterizerState()
{

}

RasterizerState::~RasterizerState()
{
	for (auto& pState : m_RasterizerStates)
	{
		SafeRelease(pState.second);
	}
	m_RasterizerStates.clear();
}

void RasterizerState::SetFillMode(FillMode fillMode)
{
	if (fillMode != m_FillMode)
	{
		m_FillMode = fillMode;
		m_IsDirty = true;
	}
}

void RasterizerState::SetCullMode(CullMode cullMode)
{
	if (cullMode != m_CullMode)
	{
		m_CullMode = cullMode;
		m_IsDirty = true;
	}
}

void RasterizerState::SetLineAntialias(bool lineAntiAlias)
{
	if (lineAntiAlias != m_LineAntiAlias)
	{
		m_LineAntiAlias = lineAntiAlias;
		m_IsDirty = true;
	}
}

void RasterizerState::SetScissorEnabled(bool enabled)
{
	if (enabled != m_ScissorEnabled)
	{
		m_ScissorEnabled = enabled;
		m_IsDirty = true;
	}
}

void RasterizerState::SetMultisampleEnabled(bool enabled)
{
	if (enabled != m_MultisampleEnabled)
	{
		m_MultisampleEnabled = enabled;
		m_IsDirty = true;
	}
}

