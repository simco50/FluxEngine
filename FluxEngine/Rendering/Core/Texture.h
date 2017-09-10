#pragma once
class Texture
{
public:
	Texture(ID3D11Resource* pTexture, ID3D11ShaderResourceView* pTextureSRV);
	~Texture();

	ID3D11Resource* GetResource() const { return m_pTexture; }
	ID3D11ShaderResourceView* GetResourceView() const { return m_pTextureSRV; }

	const int& GetWidth() const { return m_Width; }
	const int& GetHeight() const { return m_Height; }

private:
	ID3D11Resource* m_pTexture;
	ID3D11ShaderResourceView* m_pTextureSRV;
	int m_Width;
	int m_Height;
};

