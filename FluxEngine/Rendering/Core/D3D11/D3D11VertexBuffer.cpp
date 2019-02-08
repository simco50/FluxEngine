#include "FluxEngine.h"
#include "../VertexBuffer.h"
#include "../Graphics.h"
#include "D3D11GraphicsImpl.h"

void VertexBuffer::Create(int vertexCount, std::vector<VertexElement>& elements, bool dynamic)
{
	AUTOPROFILE(VertexBuffer_Create);

	Release();

	SetVertexSize(elements);
	UpdateOffsets(elements);
	m_Elements = elements;
	m_Size = m_ElementStride * vertexCount;
	m_ElementCount = vertexCount;
	m_Dynamic = dynamic;

	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = m_Size;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateBuffer(&desc, nullptr, (ID3D11Buffer**)&m_pResource));
}