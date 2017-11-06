#pragma once

class VertexBuffer;
class IndexBuffer;
struct VertexElement;

class MeshFilter
{
public:
	MeshFilter();
	~MeshFilter();

	DELETE_COPY(MeshFilter)

	//Creates vertex- and indexbuffer according to the element desc
	void CreateBuffers(Graphics* pGraphics, vector<VertexElement>& elementDesc);

	//Returns the vertexbuffer at the specified slot
	VertexBuffer* GetVertexBuffer() const { return m_pVertexBuffer.get(); }
	IndexBuffer* GetIndexBuffer() const { return m_pIndexBuffer.get(); }
	bool HasIndexBuffer() const { return m_pIndexBuffer.get() != nullptr; }

	int GetIndexCount() const { return m_IndexCount; }
	int GetVertexCount() const { return m_VertexCount; }

	struct VertexData
	{
		int Stride = 0;
		int Count = 0;
		void* pData;
	};

	//Return the vertex data of the given semantic
	VertexData& GetVertexData(const string& semantic);
	//Same as above but without range checking
	VertexData& GetVertexDataUnsafe(const string& semantic);
	bool HasData(const string& semantic) const;

private:
	friend class MeshLoader;

	string m_MeshName;
	bool m_BuffersInitialized = false;
	unique_ptr<VertexBuffer> m_pVertexBuffer;
	unique_ptr<IndexBuffer> m_pIndexBuffer;

	int m_IndexCount = 0;
	int m_VertexCount = 0;

	map<string, VertexData> m_VertexData;
};

