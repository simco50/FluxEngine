#include "FluxEngine.h"
#include "../VertexBuffer.h"
#include "D3D11GraphicsImpl.h"
#include "../Graphics.h"

const char* VertexElement::GetSemanticOfType(VertexElementSemantic semantic)
{
	switch (semantic)
	{
	case VertexElementSemantic::POSITION:
		return "POSITION";
	case VertexElementSemantic::NORMAL:
		return "NORMAL";
	case VertexElementSemantic::BINORMAL:
		return "BINORMAL";
	case VertexElementSemantic::TANGENT:
		return "TANGENT";
	case VertexElementSemantic::TEXCOORD:
		return "TEXCOORD";
	case VertexElementSemantic::COLOR:
		return "COLOR";
	case VertexElementSemantic::BLENDWEIGHTS:
		return "BLENDWEIGHT";
	case VertexElementSemantic::BLENDINDICES:
		return "BLENDINDEX";
	case VertexElementSemantic::OBJECTINDEX:
		return "OBJECTINDEX";
	case VertexElementSemantic::MAX_VERTEX_ELEMENT_SEMANTICS:
	default:
		FLUX_LOG(Warning, "[VertexElement::GetSemanticOfType()] Invalid semantic!");
		return "INVALID";
	}
}

DXGI_FORMAT VertexElement::GetFormatOfType(VertexElementType type)
{
	switch (type)
	{
	case VertexElementType::FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case VertexElementType::UBYTE4:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case VertexElementType::UBYTE4_NORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case VertexElementType::INT:
		return DXGI_FORMAT_R32_SINT;
	case VertexElementType::FLOAT2:
		return DXGI_FORMAT_R32G32_FLOAT;
	case VertexElementType::FLOAT3:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case VertexElementType::FLOAT4:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case VertexElementType::UINT4:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case VertexElementType::INT4:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case VertexElementType::MAX_VERTEX_ELEMENT_TYPES:
	default:
		FLUX_LOG(Warning, "[VertexElement::GetFormatOfType()] Invalid vertex type!");
		return (DXGI_FORMAT)0;
	}
}

void VertexBuffer::Create(int vertexCount, std::vector<VertexElement>& elements, bool dynamic)
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

	HR(m_pGraphics->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)m_pBuffer, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &mappedData));
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