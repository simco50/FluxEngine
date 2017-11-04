#include "stdafx.h"
#include "MeshLoader.h"
#include "Rendering/MeshFilter.h"

MeshLoader::MeshLoader()
{}

MeshLoader::~MeshLoader()
{}

MeshFilter* MeshLoader::LoadContent(const string& assetFile)
{
	unsigned int pointPos = (unsigned int)assetFile.rfind(L'.') + 1;
	string extension = assetFile.substr(pointPos, assetFile.length() - pointPos);
	if(extension != "flux")
	{
		stringstream stream;
		stream << "MeshLoader::LoadContent() -> '" << assetFile << "' has a wrong file extension";
		FLUX_LOG(ERROR, stream.str());
		return nullptr;
	}

	MeshFilter* pMeshFilter = new MeshFilter();

	unique_ptr<IFile> pFile = FileSystem::GetFile(assetFile);
	if (pFile == nullptr)
		return nullptr;
	if (!pFile->Open(FileMode::Read, ContentType::Binary))
		return nullptr;

	string magic = pFile->ReadSizedString();
	char minVersion, maxVersion;
	*pFile >> minVersion >> maxVersion;
	UNREFERENCED_PARAMETER(maxVersion);
	if(minVersion != SE_VERSION)
	{
		stringstream stream;
		stream << "MeshLoader::LoadContent() File '" << assetFile << "' version mismatch: Expects v" << SE_VERSION << ".0 but is v" << (int)minVersion << ".0";
		FLUX_LOG(ERROR, stream.str());
	}
	
	for(;;)
	{
		string block = pFile->ReadSizedString();
		for (char& c : block)
			c = (char)toupper(c);
		if (block == "END")
			break;

		unsigned int length, stride;
		*pFile >> length >> stride;

		pMeshFilter->GetVertexDataUnsafe(block).pData = new char[length * stride];
		pMeshFilter->GetVertexDataUnsafe(block).Count = length;
		pMeshFilter->GetVertexDataUnsafe(block).Stride = stride;
		pFile->Read(length * stride, (char*)pMeshFilter->GetVertexDataUnsafe(block).pData);
	}

	pFile->Close();

	pMeshFilter->m_VertexCount = pMeshFilter->GetVertexData("POSITION").Count;
	pMeshFilter->m_IndexCount = pMeshFilter->GetVertexData("INDEX").Count;
	return pMeshFilter;
}

void MeshLoader::Destroy(MeshFilter* objToDestroy)
{
	SafeDelete(objToDestroy);
}
