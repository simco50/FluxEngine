#include "stdafx.h"
#include "TextureLoader.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Core/Graphics.h"

TextureLoader::TextureLoader()
{
}


TextureLoader::~TextureLoader()
{
}

Texture* TextureLoader::LoadContent(const string& assetFile)
{
	unique_ptr<IFile> pFile = FileSystem::GetFile(assetFile);
	if (!pFile->Open(FileMode::Read))
		return nullptr;
	vector<char> buffer;
	pFile->ReadAllBytes(buffer);
	pFile->Close();

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
	if (extension == "dds")
	{
		HR(LoadFromDDSMemory(buffer.data(), buffer.size(), DDS_FLAGS_NONE, &metaData, image));
	}
	else if (extension == "tga")
	{
		HR(LoadFromTGAMemory(buffer.data(), buffer.size(), &metaData, image));
	}
	else
	{
		HR(LoadFromWICMemory(buffer.data(), buffer.size(), WIC_FLAGS_NONE, &metaData, image));
	}

	ID3D11Resource* pTexture;
	ID3D11ShaderResourceView* pSRV;
	
	HR(CreateTexture(m_pGraphics->GetDevice(), image.GetImages(), image.GetImageCount(), metaData, &pTexture));
	HR(CreateShaderResourceView(m_pGraphics->GetDevice(), image.GetImages(), image.GetImageCount(), metaData, &pSRV));



	return new Texture(m_pGraphics, pTexture, pSRV);
}

void TextureLoader::Destroy(Texture* objToDestroy)
{
	SafeDelete(objToDestroy);
}