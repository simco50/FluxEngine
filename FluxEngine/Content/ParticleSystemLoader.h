#pragma once
#include "ResourceLoader.h"

struct ParticleSystem;

class ParticleSystemLoader : public ResourceLoader<ParticleSystem>
{
public:
	ParticleSystemLoader();
	~ParticleSystemLoader();

	ParticleSystem* LoadContent(const string& assetFile) override;
	void Destroy(ParticleSystem* objToDestroy) override;

private:
	static const int VERSION = 2;
};

