#pragma once

class Graphics;

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

	static char* GetSemanticOfType(VertexElementSemantic semantic);

	static DXGI_FORMAT GetFormatOfType(VertexElementType type);

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
		}
		return 0;
	}
};

class VertexBuffer
{
public:
	VertexBuffer(Graphics* pGraphics);
	~VertexBuffer();

	DELETE_COPY(VertexBuffer)

	void Create(const int vertexCount, std::vector<VertexElement>& elements, bool dynamic = false);
	void SetData(void* pData);

	void* GetBuffer() const { return m_pBuffer; }

	void* Map(bool discard);
	void Unmap();

	unsigned int GetVertexStride() const { return m_VertexStride; }
	unsigned int GetVertexCount() const { return m_VertexCount; }
	const std::vector<VertexElement>& GetElements() const { return m_Elements; }

	unsigned long long GetBufferHash() const { return m_BufferHash; }

private:
	void Release();

	void SetVertexSize(const std::vector<VertexElement>& elements);
	void UpdateOffsets(std::vector<VertexElement>& elements);

	void* m_pBuffer = nullptr;

	bool m_Dynamic = false;
	bool m_Mapped = false;
	std::vector<VertexElement> m_Elements;

	unsigned int m_VertexCount = 0;
	unsigned int m_VertexStride = 0;

	Graphics* m_pGraphics;

	unsigned long long m_BufferHash = 0;
};

