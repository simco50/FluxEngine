#pragma once
#include "ResourceLoader.h"
#include "Graphics/Texture.h"
class Texture;

class TextureLoader : public ResourceLoader<Texture>
{
public:
	TextureLoader();
	~TextureLoader();

	Texture* LoadContent(const wstring& assetFile) override;
	void Destroy(Texture* objToDestroy) override;
};

