#include "FluxEngine.h"
#include "CommandContext.h"
#include "PipelineState.h"
#include "Graphics.h"

/////////Command Context
////////////////////////////////////////////

CommandContext::CommandContext(Graphics* pGraphics, void* pCommandContext)
	: m_pGraphics(pGraphics),
	m_pCommandList(pCommandContext)
{
	
}

CommandContext::~CommandContext()
{

}

bool CommandContext::SetShaderParameter(StringHash hash, const void* pData)
{
	return m_pPipelineState->SetParameter(hash, pData);
}

bool CommandContext::SetShaderParameter(StringHash hash, const void* pData, int stride, int count)
{
	return m_pPipelineState->SetParameter(hash, pData, stride * count);
}

bool CommandContext::SetShaderParameter(StringHash hash, float value)
{
	return SetShaderParameter(hash, &value, sizeof(float), 1);
}

bool CommandContext::SetShaderParameter(StringHash hash, int value)
{
	return SetShaderParameter(hash, &value, sizeof(int), 1);
}

bool CommandContext::SetShaderParameter(StringHash hash, const Vector2& value)
{
	return SetShaderParameter(hash, &value, sizeof(Vector2), 1);
}

bool CommandContext::SetShaderParameter(StringHash hash, const Vector3& value)
{
	return SetShaderParameter(hash, &value, sizeof(Vector3), 1);
}

bool CommandContext::SetShaderParameter(StringHash hash, const Vector4& value)
{
	return SetShaderParameter(hash, &value, sizeof(Vector4), 1);
}

bool CommandContext::SetShaderParameter(StringHash hash, const Color& value)
{
	return SetShaderParameter(hash, &value, sizeof(Color), 1);
}

bool CommandContext::SetShaderParameter(StringHash hash, const Matrix& value)
{
	return SetShaderParameter(hash, &value, sizeof(Matrix), 1);
}

/////////Graphics Command Context
////////////////////////////////////////////

GraphicsCommandContext::GraphicsCommandContext(Graphics* pGraphics, void* pCommandList)
	: CommandContext(pGraphics, pCommandList)
{
	m_pPipelineState = std::make_unique<GraphicsPipelineState>(pGraphics);
	m_CurrentViewport.Left = 0;
	m_CurrentViewport.Right = (float)pGraphics->GetWindowWidth();
	m_CurrentViewport.Top = 0;
	m_CurrentViewport.Bottom = (float)pGraphics->GetWindowHeight();
}

GraphicsPipelineState* GraphicsCommandContext::GetGraphicsPipelineState() const
{
	return static_cast<GraphicsPipelineState*>(m_pPipelineState.get());
}

/////////Compute Command Context
////////////////////////////////////////////

ComputeCommandContext::ComputeCommandContext(Graphics* pGraphics, void* pCommandList)
	: CommandContext(pGraphics, pCommandList)
{

}