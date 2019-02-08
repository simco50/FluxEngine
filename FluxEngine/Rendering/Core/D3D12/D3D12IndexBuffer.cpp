#include "FluxEngine.h"
#include "../IndexBuffer.h"
#include "D3D12GraphicsImpl.h"
#include "../Graphics.h"
#include "d3dx12.h"

void IndexBuffer::Create(int indexCount, bool smallIndexStride, bool dynamic /*= false*/)
{
	AUTOPROFILE(IndexBuffer_Create);
	SafeRelease(m_pResource);

	m_IndexCount = indexCount;
	m_SmallIndexStride = smallIndexStride;
	m_Dynamic = dynamic;

	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Width = indexCount * (smallIndexStride ? 2 : 4);
	desc.Height = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MipLevels = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS((ID3D12Resource**)&m_pResource)));
}

void IndexBuffer::SetData(void* pData)
{
	
}

void* IndexBuffer::Map(bool discard)
{
	return nullptr;
}

void IndexBuffer::Unmap()
{
	
}