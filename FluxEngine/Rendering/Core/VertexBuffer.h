#pragma once

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
	VertexBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~VertexBuffer();

	DELETE_COPY(VertexBuffer)

	void Create(const int vertexCount, vector<VertexElement>& elements, bool dynamic = false);
	void SetData(void* pData);

	void* GetBuffer() const { return m_pBuffer; }

	void* Map(bool discard);
	void Unmap();

	unsigned int GetVertexStride() const { return m_VertexStride; }
	unsigned int GetVertexCount() const { return m_VertexCount; }
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

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
};

