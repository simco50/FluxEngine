#include "FluxEngine.h"
#include "../VertexBuffer.h"
#include "D3D12GraphicsImpl.h"
#include "../Graphics.h"

void VertexBuffer::Create(int vertexCount, std::vector<VertexElement>& elements, bool dynamic)
{
	AUTOPROFILE(VertexBuffer_Create);

	Release();

	SetVertexSize(elements);
	UpdateOffsets(elements);
	m_Elements = elements;

	m_ElementCount = vertexCount;
	m_Dynamic = dynamic;
	m_Size = vertexCount * m_ElementStride;

	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(m_Size);
	D3D12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	
	HR(m_pGraphics->GetImpl()->GetDevice()->CreateCommittedResource(
		&props, 
		D3D12_HEAP_FLAG_NONE, 
		&desc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr, 
		IID_PPV_ARGS((ID3D12Resource**)&m_pResource)));

	m_GpuHandle = static_cast<ID3D12Resource*>(m_pResource)->GetGPUVirtualAddress();
}