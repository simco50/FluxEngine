#pragma once

class Material;

class MeshFilter
{
public:
	MeshFilter();
	~MeshFilter();

	void Initialize(GameContext* pGameContext);
	void CreateBuffers(const ILDesc* pILDesc);
	void CreateBuffers(Material* pMaterial);

	ID3D11Buffer* GetVertexBuffer() const { return m_pVertexBuffer.Get(); }
	ID3D11Buffer* GetIndexBuffer() const { return m_pIndexBuffer.Get(); }


	int IndexCount() const { return m_IndexCount; }
	int VertexCount() const { return m_VertexCount; }

	struct VertexData
	{
		int Stride = 0;
		int Count = 0;
		void* pData;
	};
	VertexData& GetVertexData(const string& semantic);
	VertexData& GetVertexDataUnsafe(const string& semantic);
	bool HasData(const string& semantic) const;

private:
	friend class MeshLoader;

	GameContext* m_pGameContext = nullptr;

	bool m_BuffersInitialized = false;
	Unique_COM<ID3D11Buffer> m_pVertexBuffer = nullptr;
	Unique_COM<ID3D11Buffer> m_pIndexBuffer = nullptr;

	int m_IndexCount = 0;
	int m_VertexCount = 0;
	string m_FilePath;

	map<string, VertexData> m_VertexData;
	void* m_pVertexDataStart = nullptr;
};

