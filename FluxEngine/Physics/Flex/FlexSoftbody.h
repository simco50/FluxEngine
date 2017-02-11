#pragma once
#include "FlexBody.h"

class FlexSoftbodyMaterial;
class Texture;
class FlexSystem;

namespace FlexHelper
{
	struct FlexMeshInstance;
}

struct SoftbodyDesc
{
	//Softbody
	int Phase = -1;
	float Radius = -1;
	float VolumeSampling = -1;
	float SurfaceSampling = -1;
	float ClusterSpacing = -1;
	float ClusterRadius = -1;
	float ClusterStiffness = -1;
	float LinkRadius = -1;
	float LinkStiffness = -1;

	//Skinning
	float SkinningFalloff = -1;
	float SkinningMaxDistance = -1;

	bool IsValid() const
	{
		if (Phase == -1) return false;
		if (Radius == -1) return false;
		if (VolumeSampling == -1) return false;
		if (SurfaceSampling == -1) return false;
		if (ClusterSpacing == -1) return false;
		if (ClusterRadius == -1) return false;
		if (ClusterStiffness == -1) return false;
		if (LinkRadius == -1) return false;
		if (LinkStiffness == -1) return false;
		if (SkinningFalloff == -1) return false;
		if (SkinningMaxDistance == -1) return false;

		return true;
	}
};

class FlexSoftbody : public FlexBody
{
public:
	FlexSoftbody(const wstring &filePath, SoftbodyDesc* desc, FlexSystem* pFlexSystem);
	~FlexSoftbody();

	void Initialize();
	void Update();
	void Render();

	void SetTexture(const wstring& filePath);

private:
	struct FlexSkinVertex
	{
		float weights[4];
		int indices[4];
	};

	void CreateSkinningBuffer();
	void LoadAndCreateBody();
	void CreateSoftbody();

	static const int MAX_CLUSTERS = 200;

	SoftbodyDesc* m_pSoftbodyDesc;
	FlexSoftbodyMaterial* m_pMaterial = nullptr;
	Unique_COM<ID3D11Buffer> m_pSkinVertexBuffer;
};

