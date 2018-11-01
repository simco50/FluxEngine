#pragma once
#include "../..\Components\ComponentBase.h"

class FlexSystem;

class FlexTriangleMeshCollider : public ComponentBase
{
public:
	FlexTriangleMeshCollider(FlexSystem* pFlexSystem);
	~FlexTriangleMeshCollider();

	void Initialize();
	void Update();
	void Render();

private:
	TransformComponent* m_pTransformComponent = nullptr;
	FlexSystem* m_pFlexSystem;
	int m_ShapeIdx = -1;

	BoundingBox m_BoundingBox;
};

