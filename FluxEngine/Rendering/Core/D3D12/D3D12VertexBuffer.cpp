#include "FluxEngine.h"
#include "../VertexBuffer.h"
#include "D3D12GraphicsImpl.h"
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