#include "stdafx.h"
#include "TextureLoader.h"
#include "Rendering/Core/Texture.h"

TextureLoader::TextureLoader()
{
}


TextureLoader::~TextureLoader()
{
}

Texture* TextureLoader::LoadContent(const string& assetFile)
{
	TexMetadata metaData;
	ScratchImage image;

	size_t pointPos = assetFile.rfind(L'.');
	if (pointPos == wstring::npos)
	{
		stringstream stream;
		stream << "TextureLoader::LoadContent() -> File '" << assetFile << "' has a wrong extension" << endl;
		FLUX_LOG(ERROR, stream.str());
		return nullptr;
	}
	++pointPos;
	string extension = assetFile.substr(pointPos, assetFile.length() - pointPos);

	wstring path = wstring(assetFile.begin(), assetFile.end());
	if (extension == "dds")
	{
		HR(LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, &metaData, image));
	}
	else if (extension == "tga")
	{
		HR(LoadFromTGAFile(path.c_str(), &metaData, image));
	}
	else
	{
		HR(LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, &metaData, image));
	}

	ID3D11Resource* pTexture;
	ID3D11ShaderResourceView* pSRV;
	
	HR(CreateTexture(m_pDevice, image.GetImages(), image.GetImageCount(), metaData, &pTexture));
	HR(CreateShaderResourceView(m_pDevice, image.GetImages(), image.GetImageCount(), metaData, &pSRV));



	return new Texture(pTexture, pSRV);
}

void TextureLoader::Destroy(Texture* objToDestroy)
{
	SafeDelete(objToDestroy);
}