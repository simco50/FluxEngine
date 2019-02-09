#include "FluxEngine.h"
#include "../PipelineState.h"
#include "../Graphics.h"
#include "D3D12GraphicsImpl.h"

class GraphicsPipelineStateImpl
{

};

GraphicsPipelineState::GraphicsPipelineState(Graphics* pGraphics)
	: PipelineState(pGraphics), m_pImpl(std::make_unique<GraphicsPipelineStateImpl>())
{
}

GraphicsPipelineState::~GraphicsPipelineState()
{
}