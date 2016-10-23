#pragma once
#include "ResourceLoader.h"
class Texture;

class TextureLoader : public ResourceLoader<Texture>
{
public:
	TextureLoader();
	~TextureLoader();

	Texture* LoadContent(const wstring& assetFile) override;
	void Destroy(Texture* objToDestroy) override;
};

