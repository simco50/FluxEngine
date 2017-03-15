#pragma once
#include "ResourceLoader.h"

class EffectLoader : public ResourceLoader<ID3DX11Effect>
{
public:
	EffectLoader(void);
	virtual ~EffectLoader(void);

protected:
	virtual ID3DX11Effect* LoadContent(const string& assetFile);
	virtual void Destroy(ID3DX11Effect* objToDestroy);

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	EffectLoader( const EffectLoader &obj);
	EffectLoader& operator=( const EffectLoader& obj);
};

