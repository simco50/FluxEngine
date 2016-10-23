#include "stdafx.h"
#include "TextureLoader.h"
#include "../Graphics/Texture.h"

TextureLoader::TextureLoader()
{
}


TextureLoader::~TextureLoader()
{
}

Texture* TextureLoader::LoadContent(const wstring& assetFile)
{
	TexMetadata metaData;
	unique_ptr<ScratchImage> image = make_unique<ScratchImage>();

	size_t pointPos = assetFile.rfind(L'.');
	if (pointPos == wstring::npos)
	{
		wstringstream stream;
		stream << L"TextureLoader::LoadContent() -> File '" << assetFile << "' has a wrong extension" << endl;
		DebugLog::Log(stream.str(), LogType::ERROR);
		return nullptr;
	}
	++pointPos;
	wstring extension = assetFile.substr(pointPos, assetFile.length() - pointPos);

	if (extension == L"dds")
	{
		HR(LoadFromDDSFile(assetFile.c_str(), DDS_FLAGS_NONE, &metaData, *image));
	}
	else if (extension == L"tga")
	{
		HR(LoadFromTGAFile(assetFile.c_str(), &metaData, *image));
	}
	else
	{
		HR(LoadFromWICFile(assetFile.c_str(), WIC_FLAGS_NONE, &metaData, *image));
	}

	ID3D11Resource* pTexture;
	ID3D11ShaderResourceView* pSRV;

	HR(CreateTexture(m_pDevice, image->GetImages(), image->GetImageCount(), metaData, &pTexture));
	HR(CreateShaderResourceView(m_pDevice, image->GetImages(), image->GetImageCount(), metaData, &pSRV));

	return new Texture(pTexture, pSRV);
}

void TextureLoader::Destroy(Texture* objToDestroy)
{
	SafeDelete(objToDestroy);
}