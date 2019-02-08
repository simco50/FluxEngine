#include "FluxEngine.h"
#include "../VertexBuffer.h"
#include "D3D12GraphicsImpl.h"
#include "../Graphics.h"
#include "d3dx12.h"

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
		checkf(false, "[VertexElement::GetSemanticOfType()] Invalid semantic!");
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
		checkf(false, "[VertexElement::GetFormatOfType()] Invalid vertex type!");
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

	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Width = m_VertexStride * vertexCount;
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

void VertexBuffer::SetData(void* pData)
{
	
}

void* VertexBuffer::Map(bool discard)
{
	return nullptr;
}

void VertexBuffer::Unmap()
{

}