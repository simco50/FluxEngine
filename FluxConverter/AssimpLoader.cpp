#include "stdafx.h"
#include "AssimpLoader.h"

#pragma comment(lib, "assimp.lib") 

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define ADD_DATA(semantic, source, count){\
\
m_VertexData[#semantic].pData = new char[count * sizeof(source[0])]; \
m_VertexData[#semantic].Count = count; \
m_VertexData[#semantic].Stride = sizeof(source[0]); \
memcpy(m_VertexData[#semantic].pData, source, m_VertexData[#semantic].Count * m_VertexData[#semantic].Stride);\
}

AssimpLoader::AssimpLoader()
{
}


AssimpLoader::~AssimpLoader()
{
}

void AssimpLoader::Load(const string& filePath)
{
	cout << "Loading file into memory" << endl;

	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(filePath,
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

	if (!pScene)
	{
		stringstream stream;
		stream << "Error loading mesh at '" << filePath << "' Scene Invalid";
		throw(stream.str());
	}

	if (pScene->mNumMeshes == 0)
	{
		stringstream stream;
		stream << "Error loading mesh at '" << filePath << "' Scene has no mesh(es)";
		throw(stream.str());
	}

	for (size_t i = 0; i < pScene->mNumMeshes; i++)
	{
		aiMesh* pMesh = pScene->mMeshes[i];

		ADD_DATA(POSITION, pMesh->mVertices, pMesh->mNumVertices)
		if (pMesh->HasNormals())
			ADD_DATA(NORMAL, pMesh->mNormals, pMesh->mNumVertices)
		if (pMesh->HasTextureCoords(0))
		{
			vector<Vector2> texCoord(pMesh->mNumVertices);
			for (size_t j = 0; j < texCoord.size(); j++)
				texCoord[j] = Vector2(pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y);
			ADD_DATA(TEXCOORD, texCoord.data(), texCoord.size())
		}
		if (pMesh->HasVertexColors(0))
		{
			ADD_DATA(COLOR, pMesh->mColors[0], pMesh->mNumVertices)
		}
		if(pMesh->HasTangentsAndBitangents())
			ADD_DATA(TANGENT, pMesh->mTangents, pMesh->mNumVertices)

		vector<DWORD> indices(pMesh->mNumFaces * 3);

		int idx = 0;
		for (size_t j = 0; j < pMesh->mNumFaces; j++)
		{
			aiFace* pFace = &pMesh->mFaces[j];
			if (pFace->mNumIndices != 3)
				throw(string("Face has more/less than 3 vertices. Not triangulated!"));
			indices[idx] = pFace->mIndices[0];
			indices[idx + 1] = pFace->mIndices[1];
			indices[idx + 2] = pFace->mIndices[2];
			idx += 3;
		}
		ADD_DATA(INDEX, indices.data(), indices.size());
	}

	cout << "Done" << endl;
}
