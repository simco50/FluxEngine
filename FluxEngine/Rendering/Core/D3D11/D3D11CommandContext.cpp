#include "FluxEngine.h"
#include "../CommandContext.h"
#include "Math/MathHelp.h"
#include "D3D11GraphicsImpl.h"
#include "../VertexBuffer.h"
#include "../Graphics.h"
#include "../IndexBuffer.h"
#include "../Texture.h"
#include "../StructuredBuffer.h"
#include "../../Geometry.h"
#include "../../Renderer.h"
#include "../ConstantBuffer.h"
#include "../ShaderVariation.h"
#include "../PipelineState.h"
#include "../RenderTarget.h"
#include "../Texture2D.h"

/////////Command Context
////////////////////////////////////////////


/////////Graphics Command Context
////////////////////////////////////////////

void GraphicsCommandContext::PrepareDraw()
{
	AUTOPROFILE(GraphicsCommandContext_PrepareDraw);

	FlushRenderTargetChanges(false);
	FlushSRVChanges(false);

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();

	if (m_ScissorRectDirty)
	{
		AUTOPROFILE(GraphicsCommandContext_PrepareDraw_SetScissorRect);

		D3D11_RECT rect = {
			(LONG)m_CurrentScissorRect.Left,
			(LONG)m_CurrentScissorRect.Top,
			(LONG)m_CurrentScissorRect.Right,
			(LONG)m_CurrentScissorRect.Bottom };
		pImpl->m_pDeviceContext->RSSetScissorRects(1, &rect);
		m_ScissorRectDirty = false;
	}

	if (pImpl->m_VertexBuffersDirty)
	{
		AUTOPROFILE(GraphicsCommandContext_PrepareDraw_SetVertexBuffers);

		//Set the vertex buffers
		pImpl->m_pDeviceContext->IASetVertexBuffers(
			pImpl->m_FirstDirtyVertexBuffer,
			pImpl->m_LastDirtyVertexBuffer - pImpl->m_FirstDirtyVertexBuffer + 1,
			&pImpl->m_CurrentVertexBuffers[pImpl->m_FirstDirtyVertexBuffer],
			&pImpl->m_CurrentStrides[pImpl->m_FirstDirtyVertexBuffer],
			&pImpl->m_CurrentOffsets[pImpl->m_FirstDirtyVertexBuffer]);

		pImpl->m_FirstDirtyVertexBuffer = UINT_MAX;
		pImpl->m_LastDirtyVertexBuffer = 0;
		pImpl->m_VertexBuffersDirty = false;
	}

	GetGraphicsPipelineState()->Apply(m_CurrentVertexBuffers.data(), (int)m_CurrentVertexBuffers.size());
}

void GraphicsCommandContext::SetRenderTarget(int index, RenderTarget* pRenderTarget)
{
	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	if (index == 0 && pRenderTarget == nullptr)
	{
		m_CurrentRenderTargets[0] = nullptr;
		pImpl->m_RenderTargetsDirty = true;
	}
	else if (m_CurrentRenderTargets[index] != pRenderTarget)
	{
		m_CurrentRenderTargets[index] = pRenderTarget;

		if (pRenderTarget && pRenderTarget->GetParentTexture()->GetMultiSample() > 1)
		{
			pRenderTarget->GetParentTexture()->SetResolveDirty(true);
		}

		pImpl->m_RenderTargetsDirty = true;
	}
}

void GraphicsCommandContext::SetDepthStencil(RenderTarget* pRenderTarget)
{
	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	if (pRenderTarget != m_pCurrentDepthStencil)
	{
		m_pCurrentDepthStencil = pRenderTarget;
		pImpl->m_RenderTargetsDirty = true;
	}
}

void GraphicsCommandContext::FlushRenderTargetChanges(bool force)
{
	AUTOPROFILE(GraphicsCommandContext_FlushRenderTargetChanges);

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	if (pImpl->m_RenderTargetsDirty || force)
	{
		for (int i = 0; i < GraphicsConstants::MAX_RENDERTARGETS; ++i)
		{
			pImpl->m_RenderTargetViews[i] = m_CurrentRenderTargets[i] ? (ID3D11RenderTargetView*)m_CurrentRenderTargets[i]->GetRenderTargetView() : nullptr;
		}

		if (pImpl->m_RenderTargetViews[0] == nullptr)
		{
			pImpl->m_RenderTargetViews[0] = (ID3D11RenderTargetView*)m_pGraphics->GetDefaultRenderTarget()->GetRenderTarget()->GetRenderTargetView();
		}

		pImpl->m_pDepthStencilView = m_pCurrentDepthStencil ? (ID3D11DepthStencilView*)m_pCurrentDepthStencil->GetRenderTargetView() : (ID3D11DepthStencilView*)m_pGraphics->GetDefaultDepthStencil()->GetRenderTarget()->GetRenderTargetView();
		pImpl->m_pDeviceContext->OMSetRenderTargets(GraphicsConstants::MAX_RENDERTARGETS, pImpl->m_RenderTargetViews.data(), pImpl->m_pDepthStencilView);
		pImpl->m_RenderTargetsDirty = false;
	}
}

void GraphicsCommandContext::FlushSRVChanges(bool force)
{
	AUTOPROFILE(GraphicsCommandContext_FlushSRVChanges);

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	if ((pImpl->m_TexturesDirty || force) && pImpl->m_LastDirtyTexture - pImpl->m_FirstDirtyTexture + 1 > 0)
	{
		if (GetGraphicsPipelineState()->GetVertexShader())
		{
			pImpl->m_pDeviceContext->VSSetShaderResources(pImpl->m_FirstDirtyTexture, pImpl->m_LastDirtyTexture - pImpl->m_FirstDirtyTexture + 1, pImpl->m_ShaderResourceViews.data() + pImpl->m_FirstDirtyTexture);
			pImpl->m_pDeviceContext->VSSetSamplers(pImpl->m_FirstDirtyTexture, pImpl->m_LastDirtyTexture - pImpl->m_FirstDirtyTexture + 1, pImpl->m_SamplerStates.data() + pImpl->m_FirstDirtyTexture);
		}
		if (GetGraphicsPipelineState()->GetPixelShader())
		{
			pImpl->m_pDeviceContext->PSSetShaderResources(pImpl->m_FirstDirtyTexture, pImpl->m_LastDirtyTexture - pImpl->m_FirstDirtyTexture + 1, pImpl->m_ShaderResourceViews.data() + pImpl->m_FirstDirtyTexture);
			pImpl->m_pDeviceContext->PSSetSamplers(pImpl->m_FirstDirtyTexture, pImpl->m_LastDirtyTexture - pImpl->m_FirstDirtyTexture + 1, pImpl->m_SamplerStates.data() + pImpl->m_FirstDirtyTexture);
		}
		if (GetGraphicsPipelineState()->GetDomainShader())
		{
			pImpl->m_pDeviceContext->DSSetShaderResources(pImpl->m_FirstDirtyTexture, pImpl->m_LastDirtyTexture - pImpl->m_FirstDirtyTexture + 1, pImpl->m_ShaderResourceViews.data() + pImpl->m_FirstDirtyTexture);
			pImpl->m_pDeviceContext->DSSetSamplers(pImpl->m_FirstDirtyTexture, pImpl->m_LastDirtyTexture - pImpl->m_FirstDirtyTexture + 1, pImpl->m_SamplerStates.data() + pImpl->m_FirstDirtyTexture);
		}

		pImpl->m_TexturesDirty = false;
		pImpl->m_FirstDirtyTexture = (int)TextureSlot::MAX;
		pImpl->m_LastDirtyTexture = 0;
	}
}

void GraphicsCommandContext::SetVertexBuffer(VertexBuffer* pBuffer)
{
	SetVertexBuffers(&pBuffer, 1);
}

void GraphicsCommandContext::SetVertexBuffers(VertexBuffer** pBuffers, int bufferCount, unsigned int instanceOffset /*= 0*/)
{
	AUTOPROFILE(GraphicsCommandContext_SetVertexBuffers);

	checkf(bufferCount <= GraphicsConstants::MAX_VERTEX_BUFFERS, "Vertex buffer count exceeded");

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	for (int i = 0; i < GraphicsConstants::MAX_VERTEX_BUFFERS; ++i)
	{
		VertexBuffer* pBuffer = i >= bufferCount ? nullptr : pBuffers[i];
		bool changed = false;

		if (pBuffer)
		{
			if (m_CurrentVertexBuffers[i] != pBuffer)
			{
				m_CurrentVertexBuffers[i] = pBuffer;
				pImpl->m_CurrentOffsets[i] = pBuffer->GetElements()[0].PerInstance ? instanceOffset : 0;
				pImpl->m_CurrentStrides[i] = pBuffer->GetElementStride();
				pImpl->m_CurrentVertexBuffers[i] = (ID3D11Buffer*)pBuffer->GetResource();
				changed = true;
			}
		}
		else if (m_CurrentVertexBuffers[i])
		{
			m_CurrentVertexBuffers[i] = nullptr;
			pImpl->m_CurrentOffsets[i] = 0;
			pImpl->m_CurrentStrides[i] = 0;
			pImpl->m_CurrentVertexBuffers[i] = nullptr;
			changed = true;
		}
		if (changed)
		{
			pImpl->m_VertexBuffersDirty = true;
			pImpl->m_InputLayoutDirty = true;
			pImpl->m_FirstDirtyVertexBuffer = Math::Min((uint32)i, pImpl->m_FirstDirtyVertexBuffer);
			pImpl->m_LastDirtyVertexBuffer = Math::Max((uint32)i, pImpl->m_LastDirtyVertexBuffer);
		}
	}
}

void GraphicsCommandContext::SetIndexBuffer(IndexBuffer* pIndexBuffer)
{
	AUTOPROFILE(GraphicsCommandContext_SetIndexBuffer);

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	if (m_pCurrentIndexBuffer != pIndexBuffer)
	{
		AUTOPROFILE(Graphics_SetIndexBuffer);
		if (pIndexBuffer)
		{
			pImpl->m_pDeviceContext->IASetIndexBuffer((ID3D11Buffer*)pIndexBuffer->GetResource(), pIndexBuffer->GetElementStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		}
		else
		{
			pImpl->m_pDeviceContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
		}
		m_pCurrentIndexBuffer = pIndexBuffer;
	}
}

void GraphicsCommandContext::SetViewport(const FloatRect& rect)
{
	D3D11_VIEWPORT viewport;
	viewport.Height = rect.GetHeight();
	viewport.Width = rect.GetWidth();
	viewport.TopLeftX = rect.Left;
	viewport.TopLeftY = rect.Top;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_CurrentViewport = { viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height };

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	pImpl->m_pDeviceContext->RSSetViewports(1, &viewport);
}

void GraphicsCommandContext::SetScissorRect(bool enabled, const IntRect& rect /*= IntRect::ZERO()*/)
{
	GetGraphicsPipelineState()->SetScissorEnabled(enabled);

	if (enabled && rect != m_CurrentScissorRect)
	{
		m_CurrentScissorRect = rect;
		m_ScissorRectDirty = true;
	}
}

void GraphicsCommandContext::SetTexture(TextureSlot slot, Texture* pTexture)
{
	checkf(slot < TextureSlot::MAX, "[Graphics::SetTexture] > Can't assign a texture to a slot out of range");

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	if (pTexture && (pTexture->GetResourceView() == pImpl->m_ShaderResourceViews[(unsigned int)slot] && pTexture->GetSamplerState() == pImpl->m_SamplerStates[(unsigned int)slot]))
	{
		return;
	}

	if (pTexture)
	{
		pTexture->UpdateParameters();
		pTexture->Resolve(false);
	}

	pImpl->m_ShaderResourceViews[(size_t)slot] = pTexture ? (ID3D11ShaderResourceView*)pTexture->GetResourceView() : nullptr;
	pImpl->m_SamplerStates[(size_t)slot] = pTexture ? (ID3D11SamplerState*)pTexture->GetSamplerState() : nullptr;
	
	pImpl->m_TexturesDirty = true;
	pImpl->m_FirstDirtyTexture = Math::Min(pImpl->m_FirstDirtyTexture, (int)slot);
	pImpl->m_LastDirtyTexture = Math::Max(pImpl->m_LastDirtyTexture, (int)slot);
}

void GraphicsCommandContext::SetStructuredBuffer(TextureSlot slot, const StructuredBuffer* pBuffer)
{
	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	if (slot >= TextureSlot::MAX)
	{
		FLUX_LOG(Warning, "[Graphics::SetStructuredBuffer] > Can't assign a structuredbuffer to a slot out of range");
		return;
	}

	if (pBuffer && (pBuffer->GetView() == pImpl->m_ShaderResourceViews[(unsigned int)slot]))
	{
		return;
	}

	pImpl->m_ShaderResourceViews[(size_t)slot] = pBuffer ? (ID3D11ShaderResourceView*)pBuffer->GetView() : nullptr;
	pImpl->m_SamplerStates[(size_t)slot] = nullptr;

	pImpl->m_TexturesDirty = true;
	pImpl->m_FirstDirtyTexture = Math::Min(pImpl->m_FirstDirtyTexture, (int)slot);
	pImpl->m_LastDirtyTexture = Math::Max(pImpl->m_LastDirtyTexture, (int)slot);
}

void GraphicsCommandContext::Draw(PrimitiveType type, int vertexStart, int vertexCount)
{
	AUTOPROFILE(Graphics_Draw);
	PrepareDraw();

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	unsigned int primitiveCount = 0;
	GraphicsImpl::GetPrimitiveType(type, vertexCount, topology, primitiveCount);
	if (topology != pImpl->m_CurrentPrimitiveType)
	{
		pImpl->m_CurrentPrimitiveType = topology;
		pImpl->m_pDeviceContext->IASetPrimitiveTopology(topology);
	}

	pImpl->m_pDeviceContext->Draw(vertexCount, vertexStart);

	++m_BatchCount;
	m_PrimitiveCount += primitiveCount;
}

void GraphicsCommandContext::DrawIndexed(PrimitiveType type, int indexCount, int indexStart, int minVertex /*= 0*/)
{
	AUTOPROFILE(Graphics_DrawIndexed);
	PrepareDraw();

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	unsigned int primitiveCount = 0;
	GraphicsImpl::GetPrimitiveType(type, indexCount, topology, primitiveCount);
	if (topology != pImpl->m_CurrentPrimitiveType)
	{
		pImpl->m_CurrentPrimitiveType = topology;
		pImpl->m_pDeviceContext->IASetPrimitiveTopology(topology);
	}

	pImpl->m_pDeviceContext->DrawIndexed(indexCount, indexStart, minVertex);

	++m_BatchCount;
	m_PrimitiveCount += primitiveCount;
}

void GraphicsCommandContext::DrawIndexedInstanced(PrimitiveType type, int indexCount, int indexStart, int instanceCount, int minVertex /*= 0*/, int instanceStart /*= 0*/)
{
	AUTOPROFILE(Graphics_DrawIndexedInstanced);
	PrepareDraw();

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	unsigned int primitiveCount = 0;
	GraphicsImpl::GetPrimitiveType(type, instanceCount * indexCount, topology, primitiveCount);
	if (topology != pImpl->m_CurrentPrimitiveType)
	{
		pImpl->m_CurrentPrimitiveType = topology;
		pImpl->m_pDeviceContext->IASetPrimitiveTopology(topology);
	}

	pImpl->m_pDeviceContext->DrawIndexedInstanced(indexCount, instanceCount, indexStart, minVertex, instanceStart);

	++m_BatchCount;
	m_PrimitiveCount += primitiveCount;
}

void GraphicsCommandContext::Clear(ClearFlags clearFlags /*= ClearFlags::All*/, const Color& color /*= Color(0.15f, 0.15f, 0.15f, 1.0f)*/, float depth /*= 1.0f*/, unsigned char stencil /*= 0*/)
{
	AUTOPROFILE(Graphics_Clear);

	float width = (float)m_pGraphics->GetWindowWidth();
	float height = (float)m_pGraphics->GetWindowHeight();
	if (m_CurrentViewport.Left == 0 && m_CurrentViewport.Top == 0 && m_CurrentViewport.Right == width && m_CurrentViewport.Bottom == height)
	{
		PrepareDraw();
		GraphicsImpl* pImpl = m_pGraphics->GetImpl();
		ID3D11RenderTargetView* pRtv = pImpl->m_RenderTargetViews[0];
		ID3D11DepthStencilView* pDsv = pImpl->m_pDepthStencilView;
		if (pRtv && (clearFlags & ClearFlags::RenderTarget) == ClearFlags::RenderTarget)
		{
			pImpl->GetDeviceContext()->ClearRenderTargetView(pRtv, &color.x);
		}
		if (pDsv)
		{
			unsigned int depthClearFlags = 0;
			if ((clearFlags & ClearFlags::Depth) == ClearFlags::Depth)
			{
				depthClearFlags |= D3D11_CLEAR_DEPTH;
			}
			if ((clearFlags & ClearFlags::Stencil) == ClearFlags::Stencil)
			{
				depthClearFlags |= D3D11_CLEAR_STENCIL;
			}
			if (depthClearFlags != 0)
			{
				pImpl->m_pDeviceContext->ClearDepthStencilView(pDsv, depthClearFlags, depth, stencil);
			}
		}
	}
	else
	{
		GetGraphicsPipelineState()->SetDepthTest(CompareMode::ALWAYS);
		GetGraphicsPipelineState()->SetDepthWrite((clearFlags & ClearFlags::Depth) == ClearFlags::Depth);
		GetGraphicsPipelineState()->SetColorWrite(((clearFlags & ClearFlags::RenderTarget) == ClearFlags::RenderTarget) ? ColorWrite::ALL : ColorWrite::NONE);
		GetGraphicsPipelineState()->SetStencilTest((clearFlags & ClearFlags::Stencil) == ClearFlags::Stencil, CompareMode::ALWAYS, StencilOperation::REF, StencilOperation::KEEP, StencilOperation::KEEP, stencil, 0XFF, 0XFF);

		Geometry* quadGeometry = m_pGraphics->GetSubsystem<Renderer>()->GetQuadGeometry();

		GetGraphicsPipelineState()->ClearShaders();
		GetGraphicsPipelineState()->SetVertexShader(m_pGraphics->GetShader("Shaders/ClearFrameBuffer", ShaderType::VertexShader));
		GetGraphicsPipelineState()->SetPixelShader(m_pGraphics->GetShader("Shaders/ClearFrameBuffer", ShaderType::PixelShader));

		Matrix worldMatrix = Matrix::CreateTranslation(Vector3(0, 0, depth));

		SetShaderParameter(ShaderConstant::cColor, color);
		SetShaderParameter(ShaderConstant::cWorld, worldMatrix);

		quadGeometry->Draw(this);
	}
}

/////////Compute Command Context
////////////////////////////////////////////