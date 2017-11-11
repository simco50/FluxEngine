#include "stdafx.h"
#include "MeshLoader.h"
#include "Rendering/Mesh.h"

MeshLoader::MeshLoader()
{}

MeshLoader::~MeshLoader()
{}

Mesh* MeshLoader::LoadContent(const string& assetFile)
{
	UNREFERENCED_PARAMETER(assetFile);
	return nullptr;
}

void MeshLoader::Destroy(Mesh* objToDestroy)
{
	SafeDelete(objToDestroy);
}
