#include "FluxEngine.h"
#include "CommandContext.h"
#include "PipelineState.h"

CommandContext::CommandContext(Graphics* pGraphics, void* pCommandContext)
	: m_pGraphics(pGraphics),
	m_pCommandList(pCommandContext)
{
}

CommandContext::~CommandContext()
{

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

GraphicsCommandContext::GraphicsCommandContext(Graphics* pGraphics, void* pCommandList)
	: CommandContext(pGraphics, pCommandList)
{
	m_pPipelineState = std::make_unique<GraphicsPipelineState>(pGraphics);
}

GraphicsPipelineState* GraphicsCommandContext::GetGraphicsPipelineState() const
{
	return static_cast<GraphicsPipelineState*>(m_pPipelineState.get());
}

ComputeCommandContext::ComputeCommandContext(Graphics* pGraphics, void* pCommandList)
	: CommandContext(pGraphics, pCommandList)
{

}