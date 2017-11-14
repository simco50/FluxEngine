#pragma once
#include "Rendering/Core/GraphicsDefines.h"

class Graphics;
class IndexBuffer;
class VertexBuffer;

class Geometry
{
public:
	Geometry();
	~Geometry();

	void SetVertexBuffer(VertexBuffer* pVertexBuffer) { m_pVertexBuffer = pVertexBuffer; }
	void SetIndexBuffer(IndexBuffer* pIndexBuffer) { m_pIndexBuffer = pIndexBuffer; }

	void Draw(Graphics* pGraphics) const;

	struct VertexData
	{
		int Stride = 0;
		int Count = 0;
		void* pData;
	};

	//Return the vertex data of the given semantic
	VertexData& GetVertexData(const std::string& semantic);
	//Same as above but without range checking
	VertexData& GetVertexDataUnsafe(const std::string& semantic);
	bool HasData(const std::string& semantic) const;
	int GetDataCount(const std::string& semantic);

	void SetDrawRange(PrimitiveType type, int indexCount, int vertexCount);

	int GetVertexCount() const { return m_VertexCount; }
	int GetIndexCount() const { return m_IndexCount; }

private:
	int m_IndexCount = 0;
	int m_VertexCount = 0;
	PrimitiveType m_PrimitiveType = PrimitiveType::TRIANGLELIST;

	IndexBuffer* m_pIndexBuffer = nullptr;
	VertexBuffer* m_pVertexBuffer = nullptr;

	std::map<std::string, VertexData> m_VertexData;
};