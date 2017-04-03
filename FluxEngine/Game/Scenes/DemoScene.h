#pragma once
#include "../../Scenegraph/SceneBase.h"

class DemoScene : public SceneBase
{
public:
	DemoScene();
	~DemoScene();

	void Initialize();
	void Update();
	void LateUpdate() override;
	void Render();

private:
	GameObject* m_pGameObject = nullptr;
	
};
