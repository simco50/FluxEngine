#include "FluxEngine.h"
#include "../CommandContext.h"
#include "D3D12GraphicsImpl.h"
#include "../Graphics.h"
#include "../PipelineState.h"
#include "../VertexBuffer.h"
#include "../IndexBuffer.h"


/////////Command Context
////////////////////////////////////////////


/////////Graphics Command Context
////////////////////////////////////////////

void GraphicsCommandContext::PrepareDraw()
{
	AUTOPROFILE(GraphicsCommandContext_PrepareDraw);
	GetGraphicsPipelineState()->Apply(nullptr, 0);
}

void GraphicsCommandContext::SetRenderTarget(int index, RenderTarget* pRenderTarget)
{
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	pCommandList->OMSetRenderTargets(1, nullptr, false, nullptr);
	GetGraphicsPipelineState()->OnRenderTargetsSet(nullptr, 0, nullptr);
}

void GraphicsCommandContext::SetDepthStencil(RenderTarget* pRenderTarget)
{
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	GetGraphicsPipelineState()->OnRenderTargetsSet(nullptr, 0, nullptr);
}

void GraphicsCommandContext::FlushRenderTargetChanges(bool /*force*/)
{
	//Nop in D3D12
}

void GraphicsCommandContext::FlushSRVChanges(bool /*force*/)
{
	//Nop in D3D12
}

void GraphicsCommandContext::SetVertexBuffer(VertexBuffer* pBuffer)
{
	SetVertexBuffers(&pBuffer, 1);
}

void GraphicsCommandContext::SetVertexBuffers(VertexBuffer** pBuffers, int bufferCount, unsigned int instanceOffset /*= 0*/)
{
	AUTOPROFILE(GraphicsCommandContext_SetVertexBuffers);
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	std::array<D3D12_VERTEX_BUFFER_VIEW, GraphicsConstants::MAX_VERTEX_BUFFERS> vertexBufferViews = {};
	for (int i = 0; i < bufferCount; ++i)
	{
		D3D12_VERTEX_BUFFER_VIEW& view = vertexBufferViews[i];
		const VertexBuffer* pVertexBuffer = pBuffers[i];
		view.BufferLocation = pVertexBuffer->GetGPUHandle();
		view.SizeInBytes = pVertexBuffer->GetSize();
		view.StrideInBytes = pVertexBuffer->GetElementStride();
	}
	pCommandList->IASetVertexBuffers(0, bufferCount, vertexBufferViews.data());
}

void GraphicsCommandContext::SetIndexBuffer(IndexBuffer* pIndexBuffer)
{
	AUTOPROFILE(GraphicsCommandContext_SetIndexBuffer);
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	D3D12_INDEX_BUFFER_VIEW view = {};
	view.SizeInBytes = pIndexBuffer->GetSize();
	view.Format = pIndexBuffer->GetElementStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	view.BufferLocation = pIndexBuffer->GetGPUHandle();
	pCommandList->IASetIndexBuffer(&view);
}

void GraphicsCommandContext::SetViewport(const FloatRect& rect)
{
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = rect.Left;
	viewport.TopLeftY = rect.Top;
	viewport.Width = rect.GetWidth();
	viewport.Height = rect.GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	pCommandList->RSSetViewports(1, &viewport);
}

void GraphicsCommandContext::SetScissorRect(bool enabled, const IntRect& rect /*= IntRect::ZERO()*/)
{
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);

	if (enabled)
	{
		D3D12_RECT scissorRect = {};
		scissorRect.left = rect.Left;
		scissorRect.bottom = rect.Bottom;
		scissorRect.right = rect.Right;
		scissorRect.top = rect.Top;
		pCommandList->RSSetScissorRects(1, &scissorRect);
	}
	else
	{
		pCommandList->RSSetScissorRects(0, nullptr);
	}
}

void GraphicsCommandContext::SetTexture(TextureSlot slot, Texture* pTexture)
{

}

void GraphicsCommandContext::SetStructuredBuffer(TextureSlot slot, const StructuredBuffer* pBuffer)
{
}

void GraphicsCommandContext::Draw(int vertexStart, int vertexCount)
{
	AUTOPROFILE(Graphics_Draw);
	PrepareDraw();
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	pCommandList->DrawInstanced(vertexCount, 1, vertexStart, 0);
}

void GraphicsCommandContext::DrawIndexed(int indexCount, int indexStart, int minVertex /*= 0*/)
{
	AUTOPROFILE(Graphics_DrawIndexed);
	PrepareDraw();
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	pCommandList->DrawIndexedInstanced(indexCount, 1, indexStart, minVertex, 0);
}

void GraphicsCommandContext::DrawIndexedInstanced(int indexCount, int indexStart, int instanceCount, int minVertex /*= 0*/, int instanceStart /*= 0*/)
{
	AUTOPROFILE(Graphics_DrawIndexedInstanced);
	PrepareDraw();
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	pCommandList->DrawIndexedInstanced(indexCount, instanceCount, indexStart, minVertex, instanceStart);
}

void GraphicsCommandContext::Clear(ClearFlags clearFlags /*= ClearFlags::All*/, const Color& color /*= Color(0.15f, 0.15f, 0.15f, 1.0f)*/, float depth /*= 1.0f*/, unsigned char stencil /*= 0*/)
{
	AUTOPROFILE(Graphics_Clear);
	ID3D12GraphicsCommandList* pCommandList = static_cast<ID3D12GraphicsCommandList*>(m_pCommandList);
	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	pCommandList->ClearRenderTargetView(pImpl->GetRtv(0), &color.x, 0, nullptr);

	unsigned int depthClearFlags = 0;
	if ((clearFlags & ClearFlags::Depth) == ClearFlags::Depth)
	{
		depthClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
	}
	if ((clearFlags & ClearFlags::Stencil) == ClearFlags::Stencil)
	{
		depthClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
	}
	pCommandList->ClearDepthStencilView(pImpl->GetDsv(), (D3D12_CLEAR_FLAGS)depthClearFlags, depth, stencil, 0, nullptr);
}

/////////Compute Command Context
////////////////////////////////////////////