#pragma once

enum class VertexElementType
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

enum class VertexElementSemantic
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
		m_Type(type),
		m_Semantic(semantic),
		m_Index(index),
		m_PerInstance(perInstance),
		m_Offset(0)
	{}

	VertexElementType m_Type;
	VertexElementSemantic m_Semantic;
	unsigned char m_Index;
	bool m_PerInstance;
	unsigned m_Offset;
};

class VertexBuffer
{
public:
	VertexBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~VertexBuffer();

	void Create(const int vertexCount, const vector<VertexElement>& elements, bool dynamic = false);

	ID3D11Buffer* GetBuffer() const { return m_pBuffer.Get(); }

	void* Map(bool discard);
	void Unmap();

	unsigned int GetVertexStride() const { return m_VertexStride; }
	unsigned int GetVertexCount() const { return m_VertexCount; }

private:
	void SetVertexSize(const vector<VertexElement>& elements);

	Smart_COM::Unique_COM<ID3D11Buffer> m_pBuffer;

	bool m_Dynamic = false;
	vector<VertexElement> m_Elements;

	unsigned int m_VertexCount = 0;
	unsigned int m_VertexStride = 0;

	bool m_HardwareLocked = false;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
};

