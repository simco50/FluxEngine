#pragma once

class VertexBuffer;
class IndexBuffer;
struct VertexElement;

class MeshFilter
{
public:
	MeshFilter();
	~MeshFilter();

	//Creates vertex- and indexbuffer according to the element desc
	void CreateBuffers(Graphics* pGraphics, vector<VertexElement>& elementDesc);

	//Returns the vertexbuffer at the specified slot
	VertexBuffer* GetVertexBuffer(const unsigned int slot) const;
	const std::vector<VertexBuffer*>& GetVertexBuffers() const { return m_VertexBuffers; }
	IndexBuffer* GetIndexBuffer() const { return m_pIndexBuffer.get(); }

	int GetIndexCount() const { return m_IndexCount; }
	int GetVertexCount() const { return m_VertexCount; }
	unsigned int GetVertexBufferCount() const { return (unsigned int)m_VertexBuffers.size(); }
	bool HasIndexBuffer() const { return m_pIndexBuffer.get(); }

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

	bool m_BuffersInitialized = false;
	std::vector<VertexBuffer*> m_VertexBuffers;
	unique_ptr<IndexBuffer> m_pIndexBuffer;

	int m_IndexCount = 0;
	int m_VertexCount = 0;
	string m_FilePath;

	map<string, VertexData> m_VertexData;
};

