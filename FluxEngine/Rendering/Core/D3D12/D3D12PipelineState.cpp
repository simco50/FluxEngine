#include "FluxEngine.h"
#include "../PipelineState.h"
#include "../Graphics.h"
#include "D3D12GraphicsImpl.h"

void PipelineState::Finalize(bool& hasUpdated)
{
	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	m_IsDirty = false;
	hasUpdated = false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	
	HR(m_pGraphics->GetImpl()->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS((ID3D12PipelineState**)&m_Data)));
	
	m_IsCreated = true;
}