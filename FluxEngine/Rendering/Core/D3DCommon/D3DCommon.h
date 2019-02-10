#include "../VertexBuffer.h"

namespace D3DCommon
{
	inline void D3DBlobToVector(ID3DBlob* pBlob, std::vector<char>& buffer)
	{
		buffer.resize(pBlob->GetBufferSize());
		memcpy(buffer.data(), pBlob->GetBufferPointer(), buffer.size());
	}

	static constexpr DXGI_FORMAT GetFormatOfType(VertexElementType type)
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
}