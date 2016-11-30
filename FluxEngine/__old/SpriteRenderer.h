#pragma once
#include "../Helpers/Singleton.h"

class Texture;
class OverlordGame;
class GameScene;

class SpriteRenderer: public Singleton<SpriteRenderer>
{
public:

	void Draw(Texture* pTexture, Vector2 position, Vector4 color = (Vector4)Colors::White, Vector2 pivot = Vector2(0,0), Vector2 scale = Vector2(1,1), float rotation = 0.f, float depth = 0.f);
	void DrawImmediate(ID3D11ShaderResourceView* pSrv, Vector2 position, Vector4 color = (Vector4)Colors::White, Vector2 pivot = Vector2(0, 0), Vector2 scale = Vector2(1, 1), float rotation = 0.f);

	void MakeTransform();

private:

	struct SpriteVertex
	{
		UINT TextureId;
		Vector4 TransformData;
		Vector4 TransformData2;
		Vector4 Color;

		bool Equals(const SpriteVertex& source) const
		{
			if (source.TextureId != TextureId)return false;
			if (source.TransformData != TransformData)return false;
			if (source.TransformData2 != TransformData2)return false;
			if (source.Color != Color)return false;

			return true;
		}
	};

	SpriteRenderer();
	~SpriteRenderer();

	friend class FluxCore;
	friend class Singleton<SpriteRenderer>;

	void InitRenderer(EngineContext* pEngineContext);
	void Draw();
	void UpdateBuffer();

	vector<SpriteVertex> m_Sprites;
	vector<Texture*> m_Textures;
	UINT m_BufferSize, m_InputLayoutSize;
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3D11InputLayout* m_pInputLayout;
	ID3D11Buffer* m_pVertexBuffer, *m_pImmediateVertexBuffer;
	SpriteVertex m_ImmediateVertex;

	ID3DX11EffectMatrixVariable* m_pTransfromMatrixV;
	Matrix m_Transform;
	ID3DX11EffectVectorVariable* m_pTextureSizeV;
	ID3DX11EffectShaderResourceVariable* m_pTextureSRV;

	EngineContext* m_pEngineContext;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpriteRenderer(const SpriteRenderer& t);
	SpriteRenderer& operator=(const SpriteRenderer& t);
};

