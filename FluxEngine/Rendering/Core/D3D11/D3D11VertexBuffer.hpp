#include "D3D11GraphicsImpl.h"

void VertexBuffer::Create(const int vertexCount, std::vector<VertexElement>& elements, bool dynamic)
{
	AUTOPROFILE(VertexBuffer_Create);

	Release();

	SetVertexSize(elements);
	UpdateOffsets(elements);
	m_Elements = elements;

	m_VertexCount = vertexCount;
	m_Dynamic = dynamic;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = m_VertexStride * vertexCount;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pBuffer));
}

void VertexBuffer::SetData(void* pData)
{
	AUTOPROFILE(VertexBuffer_SetData);

	D3D11_BOX destBox;
	destBox.left = 0;
	destBox.right = m_VertexCount * m_VertexStride;
	destBox.top = 0;
	destBox.bottom = 1;
	destBox.front = 0;
	destBox.back = 1;

	m_pGraphics->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pBuffer, 0, &destBox, pData, 0, 0);
}

void* VertexBuffer::Map(bool discard)
{
	if (!m_Dynamic)
	{
		FLUX_LOG(Error, "[VertexBuffer::Map] > Vertex buffer is not dynamic");
		return nullptr;
	}

	D3D11_MAPPED_SUBRESOURCE mappedData = {};
	mappedData.pData = nullptr;

	HR(m_pGraphics->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)m_pBuffer, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &mappedData))
	void* pBuffer = mappedData.pData;

	m_Mapped = true;
	return pBuffer;
}

void VertexBuffer::Unmap()
{
	if (m_Mapped)
	{
		m_pGraphics->GetImpl()->GetDeviceContext()->Unmap((ID3D11Buffer*)m_pBuffer, 0);
		m_Mapped = false;
	}
}