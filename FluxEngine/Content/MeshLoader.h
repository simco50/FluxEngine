#pragma once
#include "ResourceLoader.h"
class MeshFilter;

class MeshLoader : public ResourceLoader<MeshFilter>
{
public:
	MeshLoader();
	~MeshLoader();

	MeshFilter* LoadContent(const string& assetFile) override;
	void Destroy(MeshFilter* objToDestroy) override;

private:
	static const int SE_VERSION = 4;
};

