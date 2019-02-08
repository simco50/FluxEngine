#include "FluxEngine.h"
#include "../IndexBuffer.h"
#include "D3D12GraphicsImpl.h"
#include "../Graphics.h"

void IndexBuffer::Create(int indexCount, bool smallIndexStride, bool dynamic /*= false*/)
{
	AUTOPROFILE(IndexBuffer_Create);
	SafeRelease(m_pResource);

	m_ElementCount = indexCount;
	m_ElementStride = smallIndexStride ? 2 : 4;
	m_Dynamic = dynamic;
	m_Size = indexCount * m_ElementStride;

	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(m_Size);

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS((ID3D12Resource**)&m_pResource)));
}