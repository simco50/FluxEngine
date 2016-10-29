#pragma once

class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();

	void Initialize(GameContext* pGameContext);
	void Update();
	void Render();

private:
	void LoadShaderVariables();
	void CreateInputLayout();
	void CreateBuffer();

	struct SpriteVertex
	{
		Vector3 Position;
		int Rotation;
		Vector2 Pivot;
		Vector2 Scale;
	};
	int m_SpriteCapacity = 20;

	Unique_COM<ID3D11InputLayout> m_pInputLayout;
	Unique_COM<ID3D11Buffer> m_pVertexBuffer;
	ID3DX11Effect* m_pEffect = nullptr;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;

	GameContext* m_pGameContext;

	vector<SpriteVertex> m_Vertices;

	ID3DX11EffectShaderResourceVariable* m_pTextureVar = nullptr;
};

