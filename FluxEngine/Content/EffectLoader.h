#pragma once
#include "ResourceLoader.h"

class EffectLoader : public ResourceLoader<ID3DX11Effect>
{
public:
	EffectLoader(void)
	{
		
	}
	virtual ~EffectLoader(void)
	{
		
	}

	EffectLoader(const EffectLoader &obj) = delete;
	EffectLoader& operator=(const EffectLoader& obj) = delete;

protected:
	virtual ID3DX11Effect* LoadContent(const string& assetFile);
	virtual void Destroy(ID3DX11Effect* objToDestroy);
};

