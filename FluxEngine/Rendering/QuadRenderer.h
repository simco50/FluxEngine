#pragma once
class Material;
class DefaultQuadMaterial;

class QuadRenderer
{
public:
	QuadRenderer(GameContext* pGameContext);
	~QuadRenderer();

	void Render(Material* pMaterial);
	void Render(ID3D11ShaderResourceView* pSRV);
	
private:

	void CreateVertexBuffer();

	GameContext* m_pGameContext;
	Unique_COM<ID3D11Buffer> m_pVertexBuffer;
	DefaultQuadMaterial* m_pDefaultMaterial = nullptr;
};