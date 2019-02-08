#include "FluxEngine.h"
#include "../IndexBuffer.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"

void IndexBuffer::Create(int indexCount, bool smallIndexStride, bool dynamic /*= false*/)
{
	AUTOPROFILE(IndexBuffer_Create);
	SafeRelease(m_pResource);

	m_ElementCount = indexCount;
	m_ElementStride = smallIndexStride ? 2 : 4;
	m_Dynamic = dynamic;
	m_Size = indexCount * m_ElementStride;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = m_Size;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pResource));
}