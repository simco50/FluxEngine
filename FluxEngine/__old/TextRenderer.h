#pragma once
#include "../Helpers/Singleton.h"

class OverlordGame;
class SpriteFont;
class GameScene;
struct TextCache;

class TextRenderer : public Singleton<TextRenderer>
{
public:

#undef DrawText
	void DrawText(SpriteFont* pFont, const wstring& text, Vector2 position, Vector4 color = (Vector4) Colors::White);

	void MakeTransform();

private:

	struct TextVertex
	{
		UINT ChannelId;
		Vector3 Position;
		Vector4 Color;
		Vector2 TexCoord;
		Vector2 CharacterDimension;
	};

	friend class Singleton<TextRenderer>;
	friend class FluxCore;

	TextRenderer();
	~TextRenderer();

	void InitRenderer(EngineContext* pEngineContext);
	void Draw();
	void UpdateBuffer();
	void CreateTextVertices(SpriteFont* pFont, const TextCache& info, TextVertex* pBuffer, int& bufferPosition);

	UINT m_BufferSize, m_InputLayoutSize, m_NumCharacters;
	Matrix m_Transform;
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3DX11EffectMatrixVariable* m_pTransfromMatrixV;
	ID3DX11EffectVectorVariable* m_pTextureSizeV;
	ID3DX11EffectShaderResourceVariable* m_pTextureSRV;

	ID3D11InputLayout* m_pInputLayout;
	ID3D11Buffer* m_pVertexBuffer;

	vector<SpriteFont*> m_SpriteFonts;

	EngineContext* m_pEngineContext = nullptr;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	TextRenderer(const TextRenderer& t);
	TextRenderer& operator=(const TextRenderer& t);
};
