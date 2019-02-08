#include "FluxEngine.h"
#include "D3D12GraphicsImpl.h"
#include "../ConstantBuffer.h"
#include "../Graphics.h"
#include "d3dx12.h"

void ConstantBuffer::SetSize(const unsigned int size)
{
	AUTOPROFILE(ConstantBuffer_SetSize);

	Release();
	m_Size = size;

	m_pShadowData = new unsigned char[m_Size];

	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Width = size;
	desc.Height = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MipLevels = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* pResource = static_cast<ID3D12Resource*>(m_pResource);

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS((ID3D12Resource**)&pResource)));

	// Describe and create a constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = pResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = size;    // CB size is required to be 256-byte aligned.
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle{};
	m_pGraphics->GetImpl()->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);

	// Map and initialize the constant buffer. We don't unmap this until the
	// app closes. Keeping things mapped for the lifetime of the resource is okay.
	// We do not intend to read from this resource on the CPU.
	CD3DX12_RANGE readRange(0, 0);
	pResource->Map(0, &readRange, reinterpret_cast<void**>(&m_pMemoryHandle));
}

void ConstantBuffer::Apply()
{

}