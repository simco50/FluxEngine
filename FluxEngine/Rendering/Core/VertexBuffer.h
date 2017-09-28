#pragma once

class Graphics;

enum class VertexElementType : unsigned char
{
	INT,
	FLOAT,
	VECTOR2,
	VECTOR3,
	VECTOR4,
	UBYTE4,
	UBYTE4_NORM,
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
	unsigned Offset;

	static char* GetSemanticOfType(VertexElementSemantic semantic)
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
		}
		FLUX_LOG(WARNING, "[VertexElement::GetSemanticOfType()] Invalid semantic!");
		return "INVALID";
	}

	static DXGI_FORMAT GetFormatOfType(VertexElementType type)
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
		case VertexElementType::VECTOR2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case VertexElementType::VECTOR3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case VertexElementType::VECTOR4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		FLUX_LOG(WARNING, "[VertexElement::GetFormatOfType()] Invalid vertex type!");
		return (DXGI_FORMAT)0;
	}

	static unsigned int GetSizeOfType(VertexElementType type)
	{
		switch (type)
		{
		case VertexElementType::FLOAT:
		case VertexElementType::UBYTE4:
		case VertexElementType::UBYTE4_NORM:
		case VertexElementType::INT:
			return 4;
		case VertexElementType::VECTOR2:
			return 8;
		case VertexElementType::VECTOR3:
			return 12;
		case VertexElementType::VECTOR4:
			return 16;
		}
		FLUX_LOG(WARNING, "[VertexElement::GetSizeOfType()] Invalid vertex type!");
		return 0;
	}
};

class VertexBuffer
{
public:
	VertexBuffer(Graphics* pGraphics);
	~VertexBuffer();

	DELETE_COPY(VertexBuffer)

	void Create(const int vertexCount, vector<VertexElement>& elements, bool dynamic = false);
	void SetData(void* pData);

	void* GetBuffer() const { return m_pBuffer; }

	void* Map(bool discard);
	void Unmap();

	unsigned int GetStride() const { return m_VertexStride; }
	unsigned int GetCount() const { return m_VertexCount; }
	const vector<VertexElement>& GetElements() const { return m_Elements; }

private:
	void Release();

	void SetVertexSize(const vector<VertexElement>& elements);
	void UpdateOffsets(vector<VertexElement>& elements);

	void* m_pBuffer = nullptr;

	bool m_Dynamic = false;
	vector<VertexElement> m_Elements;
	bool m_HardwareLocked = false;

	unsigned int m_VertexCount = 0;
	unsigned int m_VertexStride = 0;

	Graphics* m_pGraphics;
};

