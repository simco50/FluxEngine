#include "FluxEngine.h"
#include "../IndexBuffer.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"

void IndexBuffer::Create(const int indexCount, const bool smallIndexStride, bool dynamic /*= false*/)
{
	AUTOPROFILE(IndexBuffer_Create);
	SafeRelease(m_pBuffer);

	m_IndexCount = indexCount;
	m_SmallIndexStride = smallIndexStride;
	m_Dynamic = dynamic;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = indexCount * (smallIndexStride ? 2 : 4);
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pBuffer));
}

void IndexBuffer::SetData(void* pData)
{
	AUTOPROFILE(IndexBuffer_SetData);

	D3D11_BOX destBox;
	destBox.left = 0;
	destBox.right = m_IndexCount * (m_SmallIndexStride ? 2 : 4);
	destBox.top = 0;
	destBox.bottom = 1;
	destBox.front = 0;
	destBox.back = 1;

	m_pGraphics->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pBuffer, 0, &destBox, pData, 0, 0);
}

void* IndexBuffer::Map(bool discard)
{
	D3D11_MAPPED_SUBRESOURCE mappedData;
	mappedData.pData = nullptr;

	HR(m_pGraphics->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)m_pBuffer, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &mappedData))
	void* pBuffer = mappedData.pData;

	m_HardwareLocked = true;
	return pBuffer;
}

void IndexBuffer::Unmap()
{
	if (m_HardwareLocked)
	{
		m_pGraphics->GetImpl()->GetDeviceContext()->Unmap((ID3D11Buffer*)m_pBuffer, 0);
		m_HardwareLocked = false;
	}
}