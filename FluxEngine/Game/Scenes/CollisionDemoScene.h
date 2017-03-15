#pragma once
#include "../../Scenegraph/SceneBase.h"

class CollisionDemoScene : public SceneBase
{
public:
	CollisionDemoScene();
	~CollisionDemoScene();

	void Initialize();
	void Update();
	void LateUpdate() override;
	void Render();

private:

	
};
