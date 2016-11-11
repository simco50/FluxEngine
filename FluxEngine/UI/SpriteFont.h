#pragma once

struct TextCache
{
public:
	TextCache(const wstring& text, XMFLOAT2 pos, XMFLOAT4 col) :
	Text(text), Position(pos), Color(col){}

	const wstring Text;
	XMFLOAT2 Position;
	XMFLOAT4 Color;

private:
	TextCache& operator=(const TextCache &tmp);
};

struct FontMetric
{
	//BMFONT
	bool IsValid;
	wchar_t Character;
	USHORT Width;
	USHORT Height;
	short OffsetX;
	short OffsetY;
	short AdvanceX;
	UCHAR Page;
	UCHAR Channel;
	XMFLOAT2 TexCoord;
};

class Texture;
class TextRenderer;

class SpriteFont
{
public:
	SpriteFont();
	virtual ~SpriteFont();

	static bool IsCharValid(const wchar_t& character);

	static const int MAX_CHAR_ID = 255;
	static const int MIN_CHAR_ID = 0;
	static const int CHAR_COUNT = MAX_CHAR_ID - MIN_CHAR_ID + 1;

private:

	friend class SpriteFontLoader;
	friend class TextRenderer;

	FontMetric& GetMetric(const wchar_t& character){ return m_CharTable[character - MIN_CHAR_ID]; };

	FontMetric m_CharTable[CHAR_COUNT];
	vector<TextCache> m_TextCache;

	short m_FontSize;
	wstring m_FontName;
	int m_CharacterCount;
	int m_CharacterSpacing;
	int m_TextureWidth;
	int m_TextureHeight;
	int m_BufferStart, m_BufferSize;
	Texture *m_pTexture;
	bool m_IsAddedToRenderer;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpriteFont(const SpriteFont& t);
	SpriteFont& operator=(const SpriteFont& t);
};

