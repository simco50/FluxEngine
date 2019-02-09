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
#include "D3D11InputLayout.h"
#include "../ConstantBuffer.h"
#include "../ShaderVariation.h"
#include "../PipelineState.h"

//GraphicsCommandContext
void GraphicsCommandContext::PrepareDraw()
{
	AUTOPROFILE(Graphics_PrepareDraw);

	FlushRenderTargetChanges(false);
	FlushSRVChanges(false);

	m_pPipelineState->ApplyShader(ShaderType::VertexShader);
	m_pPipelineState->ApplyShader(ShaderType::PixelShader);
	m_pPipelineState->ApplyShader(ShaderType::GeometryShader);
	m_pPipelineState->ApplyShader(ShaderType::HullShader);
	m_pPipelineState->ApplyShader(ShaderType::DomainShader);

	GraphicsImpl* pImpl = m_pGraphics->GetImpl();

	bool pipelineStateUpdated = false;
	GetGraphicsPipelineState()->Finalize(pipelineStateUpdated);

	if (pipelineStateUpdated)
	{
		AUTOPROFILE(Graphics_PrepareDraw_UpdatePipelineState);
		const PipelineStateData& data = m_pPipelineState->GetData();
		pImpl->m_pDeviceContext->OMSetDepthStencilState((ID3D11DepthStencilState*)data.pDepthStencilState, GetGraphicsPipelineState()->GetStencilRef());
		pImpl->m_pDeviceContext->RSSetState((ID3D11RasterizerState*)data.pRasterizerState);
		pImpl->m_pDeviceContext->OMSetBlendState((ID3D11BlendState*)data.pBlendState, nullptr, UINT_MAX);
	}

	if (pImpl->m_VertexBuffersDirty)
	{
		AUTOPROFILE(Graphics_PrepareDraw_SetVertexBuffers);

		//Set the vertex buffers
		pImpl->m_pDeviceContext->IASetVertexBuffers(
			pImpl->m_FirstDirtyVertexBuffer,
			pImpl->m_LastDirtyVertexBuffer - pImpl->m_FirstDirtyVertexBuffer + 1,
			&pImpl->m_CurrentVertexBuffers[pImpl->m_FirstDirtyVertexBuffer],
			&pImpl->m_CurrentStrides[pImpl->m_FirstDirtyVertexBuffer],
			&pImpl->m_CurrentOffsets[pImpl->m_FirstDirtyVertexBuffer]);

		//Calculate the input element description hash to find the correct input layout
		unsigned long long hash = 0;
		for (VertexBuffer* pBuffer : m_CurrentVertexBuffers)
		{
			if (pBuffer)
			{
				hash <<= pBuffer->GetElements().size() * 10;
				hash |= pBuffer->GetBufferHash();
			}
			else
			{
				hash <<= 1;
			}
		}

		if (hash == 0)
		{
			pImpl->m_pDeviceContext->IASetInputLayout(nullptr);
		}
		else
		{
			auto pInputLayout = pImpl->m_InputLayoutMap.find(hash);
			if (pInputLayout != pImpl->m_InputLayoutMap.end())
			{
				pImpl->m_pDeviceContext->IASetInputLayout((ID3D11InputLayout*)pInputLayout->second->GetResource());
			}
			else
			{
				ShaderVariation* pVertexShader = GetGraphicsPipelineState()->GetVertexShader();
				checkf(pVertexShader, "[GraphicsCommandContext] No vertex shader set");
				std::unique_ptr<InputLayout> pNewInputLayout = std::make_unique<InputLayout>(m_pGraphics);
				pNewInputLayout->Create(m_CurrentVertexBuffers.data(), (unsigned int)m_CurrentVertexBuffers.size(), pVertexShader);
				pImpl->m_pDeviceContext->IASetInputLayout((ID3D11InputLayout*)pNewInputLayout->GetResource());
				pImpl->m_InputLayoutMap[hash] = std::move(pNewInputLayout);
			}
		}

		pImpl->m_FirstDirtyVertexBuffer = UINT_MAX;
		pImpl->m_LastDirtyVertexBuffer = 0;
		pImpl->m_VertexBuffersDirty = false;
	}

	if (m_ScissorRectDirty)
	{
		AUTOPROFILE(Graphics_PrepareDraw_SetScissorRect);

		D3D11_RECT rect = {
			(LONG)m_CurrentScissorRect.Left,
			(LONG)m_CurrentScissorRect.Top,
			(LONG)m_CurrentScissorRect.Right,
			(LONG)m_CurrentScissorRect.Bottom };
		pImpl->m_pDeviceContext->RSSetScissorRects(1, &rect);
		m_ScissorRectDirty = false;
	}
}

bool CommandContext::SetShaderParameter(StringHash hash, const void* pData)
{
	return m_pPipelineState->SetParameter(hash, pData);
}

bool CommandContext::SetShaderParameter(StringHash hash, const void* pData, int stride, int count)
{
	return m_pPipelineState->SetParameter(hash, pData);
}

//GraphicsCommandContext

void GraphicsCommandContext::SetRenderTarget(int index, RenderTarget* pRenderTarget)
{

}

void GraphicsCommandContext::SetDepthStencil(RenderTarget* pRenderTarget)
{

}

void GraphicsCommandContext::FlushRenderTargetChanges(bool force)
{

}

void GraphicsCommandContext::FlushSRVChanges(bool force)
{

}

void GraphicsCommandContext::SetVertexBuffer(VertexBuffer* pBuffer)
{
	SetVertexBuffers(&pBuffer, 1);
}

void GraphicsCommandContext::SetVertexBuffers(VertexBuffer** pBuffers, int bufferCount, unsigned int instanceOffset /*= 0*/)
{
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
			pImpl->m_FirstDirtyVertexBuffer = Math::Min((uint32)i, pImpl->m_FirstDirtyVertexBuffer);
			pImpl->m_LastDirtyVertexBuffer = Math::Max((uint32)i, pImpl->m_LastDirtyVertexBuffer);
		}
	}
}

void GraphicsCommandContext::SetIndexBuffer(IndexBuffer* pIndexBuffer)
{
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

		GetGraphicsPipelineState()->SetVertexShader(m_pGraphics->GetShader("Shaders/ClearFrameBuffer", ShaderType::VertexShader));
		GetGraphicsPipelineState()->SetPixelShader(m_pGraphics->GetShader("Shaders/ClearFrameBuffer", ShaderType::PixelShader));

		Matrix worldMatrix = Matrix::CreateTranslation(Vector3(0, 0, depth));

		SetShaderParameter(ShaderConstant::cColor, color);
		SetShaderParameter(ShaderConstant::cWorld, worldMatrix);

		quadGeometry->Draw(this);
	}
}

//ComputeCommandContext

bool ComputeCommandContext::SetComputeShader(ShaderVariation* pShader)
{
	m_pComputeShader = pShader;
	m_ComputeShaderDirty = true;
	return false;
}
