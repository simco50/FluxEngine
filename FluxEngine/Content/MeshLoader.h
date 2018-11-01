#pragma once
#include "ResourceLoader.h"
#include "Graphics/MeshFilter.h"
class MeshFilter;

class MeshLoader : public ResourceLoader<MeshFilter>
{
public:
	MeshLoader();
	~MeshLoader();

	MeshFilter* LoadContent(const wstring& assetFile) override;
	void Destroy(MeshFilter* objToDestroy) override;

private:
	static const int SE_VERSION = 4;
};

