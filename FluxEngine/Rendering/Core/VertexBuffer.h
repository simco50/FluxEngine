#pragma once
#include "GraphicsResource.h"

enum class VertexElementType : unsigned char
{
	INT,
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	UBYTE4,
	UBYTE4_NORM,
	UINT4,
	INT4,
	MAX_VERTEX_ELEMENT_TYPES
};

enum class VertexElementSemantic : unsigned char
{
	POSITION,
	NORMAL,
	BINORMAL,
	TANGENT,
	TEXCOORD,
	COLOR,
	BLENDWEIGHTS,
	BLENDINDICES,
	OBJECTINDEX,
	MAX_VERTEX_ELEMENT_SEMANTICS
};

struct VertexElement
{
	VertexElement(VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0, bool perInstance = false) :
		Type(type),
		Semantic(semantic),
		Index(index),
		PerInstance(perInstance),
		Offset(0)
	{}

	VertexElementType Type;
	VertexElementSemantic Semantic;
	unsigned char Index;
	bool PerInstance;
	unsigned int Offset;

	unsigned long long GetHash() const
	{
		unsigned long long hash = 0;
		hash |=
			(int)Type << 0
			| (int)Semantic << 3
			| Index << 7;
		return hash;
	}

	bool operator==(const VertexElement& other)
	{
		return other.GetHash() == GetHash();
	}

	static constexpr const char* GetSemanticOfType(VertexElementSemantic semantic)
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

	static constexpr unsigned int GetSizeOfType(const VertexElementType type)
	{
		switch (type)
		{
		case VertexElementType::FLOAT:
		case VertexElementType::UBYTE4:
		case VertexElementType::UBYTE4_NORM:
		case VertexElementType::INT:
			return sizeof(float);
		case VertexElementType::FLOAT2:
			return 2 * sizeof(float);
		case VertexElementType::FLOAT3:
			return 3 * sizeof(float);
		case VertexElementType::FLOAT4:
		case VertexElementType::UINT4:
		case VertexElementType::INT4:
			return 4 * sizeof(float);
		case VertexElementType::MAX_VERTEX_ELEMENT_TYPES:
		default:
			return 0;
		}
	}
};

class VertexBuffer : public GraphicsResource
{
public:
	explicit VertexBuffer(Graphics* pGraphics);
	~VertexBuffer();

	DELETE_COPY(VertexBuffer)

	void Create(int vertexCount, std::vector<VertexElement>& elements, bool dynamic = false);

	const std::vector<VertexElement>& GetElements() const { return m_Elements; }

	unsigned long long GetBufferHash() const { return m_BufferHash; }

private:
	void SetVertexSize(const std::vector<VertexElement>& elements);
	void UpdateOffsets(std::vector<VertexElement>& elements);

	std::vector<VertexElement> m_Elements;

	unsigned long long m_BufferHash = 0;
};

