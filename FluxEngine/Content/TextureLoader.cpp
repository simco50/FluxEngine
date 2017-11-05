#include "stdafx.h"
#include "TextureLoader.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Core/Graphics.h"
#include "Rendering/Core/D3D11/D3D11GraphicsImpl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "External/Stb/stb_image.h"

TextureLoader::TextureLoader()
{
}


TextureLoader::~TextureLoader()
{
}

Texture* TextureLoader::LoadContent(const string& assetFile)
{
	unique_ptr<IFile> pFile = FileSystem::GetFile(assetFile);
	if (pFile == nullptr)
		return nullptr;
	if (!pFile->Open(FileMode::Read, ContentType::Binary))
		return nullptr;
	vector<char> buffer;
	pFile->ReadAllBytes(buffer);
	pFile->Close();

	int width, height, bpp;
	unsigned char* pPixels = stbi_load_from_memory((stbi_uc*)buffer.data(), (int)buffer.size(), &width, &height, &bpp, 4);
	vector<unsigned char> pixels;
	pixels.resize(width * height * bpp);
	memcpy(pixels.data(), pPixels, pixels.size());

	Texture* pTexture = new Texture(m_pGraphics);
	pTexture->SetSize(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::STATIC, 1, nullptr);
	pTexture->SetData(pPixels);

	stbi_image_free(pPixels);
	return pTexture;
}

void TextureLoader::Destroy(Texture* objToDestroy)
{
	SafeDelete(objToDestroy);
}