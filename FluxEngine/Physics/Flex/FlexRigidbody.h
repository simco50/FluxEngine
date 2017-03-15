#pragma once
#include "FlexBody.h"

class Texture;
class FlexRigidbodyMaterial;

namespace FlexHelper
{
	struct FlexMeshInstance;
}

struct RigidbodyDesc
{
	//Softbody
	int Phase = -1;
	float Radius = -1;
	float Expand = -1;

	bool IsValid() const
	{
		if (Phase == -1) return false;
		if (Radius == -1) return false;
		if (Expand == -1) return false;
		return true;
	}
};

class FlexRigidbody : public FlexBody
{
public:
	FlexRigidbody(const string &filePath, RigidbodyDesc* desc, FlexSystem* pFlexSystem);
	~FlexRigidbody();

	void Initialize();
	void Update();
	void Render();

	void SetTexture(const string& filePath);

private:
	void CreateRigidBody();
	void LoadAndCreateBody();
	RigidbodyDesc* m_pRigidbodyDesc;

	FlexRigidbodyMaterial* m_pMaterial = nullptr;
	XMFLOAT4 m_Color;
};

