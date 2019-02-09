#include "FluxEngine.h"
#include "PipelineState.h"
#include "Graphics.h"
#include "D3D11/D3D11GraphicsImpl.h"
#include "ShaderVariation.h"

//PipelineState

PipelineState::PipelineState(Graphics* pGraphics)
	: m_pGraphics(pGraphics)
{

}

PipelineState::~PipelineState()
{

}

const PipelineStateData& PipelineState::GetData() const
{
	return m_Data;
}

void PipelineState::LoadShaderParametersOfShader(ShaderVariation* pShader)
{
	if (pShader == nullptr)
	{
		return;
	}
	const std::map<StringHash, ShaderParameter>& parameters = pShader->GetParameters();
	for (const auto& parameter : parameters)
	{
		m_ShaderParameters[parameter.first] = &parameter.second;
	}
}

//GraphicsPipelineState

GraphicsPipelineState::GraphicsPipelineState(Graphics* pGraphics)
	: PipelineState(pGraphics)
{

}

GraphicsPipelineState::~GraphicsPipelineState()
{

}

void GraphicsPipelineState::SetBlendMode(const BlendMode& blendMode, const bool alphaToCoverage)
{
	if (blendMode != m_BlendMode || alphaToCoverage != m_AlphaToCoverage)
	{
		m_BlendMode = blendMode;
		m_AlphaToCoverage = alphaToCoverage;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_BlendStateDirty = true;
	}
}

void GraphicsPipelineState::SetColorWrite(const ColorWrite colorWriteMask /*= ColorWrite::ALL*/)
{
	if (m_ColorWriteMask != colorWriteMask)
	{
		m_ColorWriteMask = colorWriteMask;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_BlendStateDirty = true;
	}
}

void GraphicsPipelineState::SetDepthEnabled(const bool enabled)
{
	if (enabled != m_DepthEnabled)
	{
		m_DepthEnabled = enabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
}

void GraphicsPipelineState::SetDepthWrite(const bool enabled)
{
	if (enabled != m_DepthWrite)
	{
		m_DepthWrite = enabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
}

void GraphicsPipelineState::SetDepthTest(const CompareMode& comparison)
{
	if (comparison != m_DepthCompareMode)
	{
		m_DepthCompareMode = comparison;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
}

void GraphicsPipelineState::SetStencilTest(bool stencilEnabled, const CompareMode mode, const StencilOperation pass, const StencilOperation fail, const StencilOperation zFail, const unsigned int stencilRef, const unsigned char compareMask, unsigned char writeMask)
{
	if (stencilEnabled != m_StencilTestEnabled)
	{
		m_StencilTestEnabled = stencilEnabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (mode != m_StencilTestMode)
	{
		m_StencilTestMode = mode;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (pass != m_StencilTestPassOperation)
	{
		m_StencilTestPassOperation = pass;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (fail != m_StencilTestFailOperation)
	{
		m_StencilTestFailOperation = fail;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (zFail != m_StencilTestZFailOperation)
	{
		m_StencilTestZFailOperation = zFail;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (stencilRef != m_StencilRef)
	{
		m_StencilRef = stencilRef;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (compareMask != m_StencilCompareMask)
	{
		m_StencilCompareMask = compareMask;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
	if (writeMask != m_StencilWriteMask)
	{
		m_StencilWriteMask = writeMask;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_DepthStencilStateDirty = true;
	}
}

void GraphicsPipelineState::SetFillMode(FillMode fillMode)
{
	if (fillMode != m_FillMode)
	{
		m_FillMode = fillMode;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

void GraphicsPipelineState::SetCullMode(CullMode cullMode)
{
	if (cullMode != m_CullMode)
	{
		m_CullMode = cullMode;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

void GraphicsPipelineState::SetLineAntialias(bool lineAntiAlias)
{
	if (lineAntiAlias != m_LineAntiAlias)
	{
		m_LineAntiAlias = lineAntiAlias;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

void GraphicsPipelineState::SetScissorEnabled(bool enabled)
{
	if (enabled != m_ScissorEnabled)
	{
		m_ScissorEnabled = enabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

void GraphicsPipelineState::SetMultisampleEnabled(bool enabled)
{
	if (enabled != m_MultisampleEnabled)
	{
		m_MultisampleEnabled = enabled;
		m_IsDirty = true;
		m_pGraphics->GetImpl()->m_RasterizerStateDirty = true;
	}
}

ComputePipelineState::ComputePipelineState(Graphics* pGraphics)
	: PipelineState(pGraphics)
{

}

ComputePipelineState::~ComputePipelineState()
{

}