#include "stdafx.h"
#include "MeshLoader.h"
#include "../Graphics/MeshFilter.h"
#include "../Helpers/BinaryReader.h"

MeshLoader::MeshLoader()
{}

MeshLoader::~MeshLoader()
{}

MeshFilter* MeshLoader::LoadContent(const string& assetFile)
{
	int pointPos = assetFile.rfind(L'.') + 1;
	string extension = assetFile.substr(pointPos, assetFile.length() - pointPos);
	if(extension != "flux")
	{
		stringstream stream;
		stream << "MeshLoader::LoadContent() -> '" << assetFile << "' has a wrong file extension";
		Console::Log(stream.str(), LogType::ERROR);
		return nullptr;
	}

	MeshFilter* pMeshFilter = new MeshFilter();
	pMeshFilter->m_FilePath = assetFile;
	unique_ptr<BinaryReader> pReader = make_unique<BinaryReader>();
	pReader->Open(assetFile);

	string magic = pReader->ReadString();
	char minVersion = pReader->Read<char>();
	char maxVersion = pReader->Read<char>();
	UNREFERENCED_PARAMETER(maxVersion);
	if(minVersion != SE_VERSION)
	{
		stringstream stream;
		stream << "MeshLoader::LoadContent() File '" << assetFile << "' version mismatch: Expects v" << SE_VERSION << ".0 but is v" << (int)minVersion << ".0";
		Console::Log(stream.str(), LogType::ERROR);
	}
	
	for(;;)
	{
		string block = pReader->ReadString();
		for (char& c : block)
			c = (char)toupper(c);
		if (block == "END")
			break;

		unsigned int length = pReader->Read<unsigned int>();
		unsigned int stride = pReader->Read<unsigned int>();

		pMeshFilter->GetVertexDataUnsafe(block).pData = new char[length * stride];
		pMeshFilter->GetVertexDataUnsafe(block).Count = length;
		pMeshFilter->GetVertexDataUnsafe(block).Stride = stride;
		pReader->Read(pMeshFilter->GetVertexDataUnsafe(block).pData, length * stride);
	}
	pMeshFilter->m_VertexCount = pMeshFilter->GetVertexData("POSITION").Count;
	pMeshFilter->m_IndexCount = pMeshFilter->GetVertexData("INDEX").Count;
	return pMeshFilter;
}

void MeshLoader::Destroy(MeshFilter* objToDestroy)
{
	SafeDelete(objToDestroy);
}
