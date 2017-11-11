#pragma once
#include "ResourceLoader.h"
class Mesh;

class MeshLoader : public ResourceLoader<Mesh>
{
public:
	MeshLoader();
	~MeshLoader();

	Mesh* LoadContent(const string& assetFile) override;
	void Destroy(Mesh* objToDestroy) override;

private:
	static const int SE_VERSION = 5;
};

