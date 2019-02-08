#include "FluxEngine.h"
#include "../GraphicsResource.h"
#include "../D3D12/D3D12GraphicsImpl.h"
#include "../Graphics.h"
#include "d3dx12.h"

void GraphicsResource::Release()
{
	SafeRelease(m_pResource);
	SafeRelease(m_pShaderResourceView);
}

bool GraphicsResource::Apply_Internal(const void* pData, int offset, int size)
{
	if (m_Dynamic)
	{
		void* pTarget = Map(pData);
		check(pTarget);
		memcpy(static_cast<char*>(pTarget) + offset, pData, size);
	}
	else
	{
		//#SimonC: todo: the temp upload buffer should come from a shared heap otherwise we have to stall the GPU because we have to keep the upload buffer in scope until we're finished
		//Create intermediate upload buffer
		ComPtr<ID3D12Resource> pIntermediate;
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);

		HR(m_pGraphics->GetImpl()->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(pIntermediate.GetAddressOf())));

		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = pData;
		subResourceData.RowPitch = size;
		subResourceData.SlicePitch = 1;
		
		ID3D12GraphicsCommandList* pTempCommandList = m_pGraphics->GetImpl()->GetTemporaryCommandList();
		pTempCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(static_cast<ID3D12Resource*>(m_pResource), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 0, D3D12_RESOURCE_BARRIER_FLAG_NONE));
		UpdateSubresources(pTempCommandList, static_cast<ID3D12Resource*>(m_pResource), pIntermediate.Get(), offset, 0, 1, &subResourceData);
		pTempCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(static_cast<ID3D12Resource*>(m_pResource), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, 0, D3D12_RESOURCE_BARRIER_FLAG_NONE));

		//#SimonC todo: Submit and wait for GPU
	}
	return true;
}

void* GraphicsResource::Map(bool discard)
{
	check(m_pResource);
	checkf(m_Dynamic, "[VertexBuffer::Map] > Vertex buffer is not dynamic");

	void* pTarget = nullptr;
	if (m_Mapped == false)
	{
		CD3DX12_RANGE range(0, 0);
		HR(static_cast<ID3D12Resource*>(m_pResource)->Map(0, &range, &pTarget));
		m_Mapped = true;
	}
	return pTarget;
}

void GraphicsResource::Unmap()
{
	if (m_Mapped)
	{
		CD3DX12_RANGE writtenRange(0, m_Size);
		static_cast<ID3D12Resource*>(m_pResource)->Unmap(0, &writtenRange);
		m_Mapped = false;
	}
}