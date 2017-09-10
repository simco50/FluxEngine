#pragma once
#include "../../Scenegraph/SceneBase.h"

class DefaultMaterial;

class DemoScene : public SceneBase
{
public:
	DemoScene();
	~DemoScene();

	void Initialize();
	void Update();
	void LateUpdate() override;

private:
	GameObject* m_pGameObject = nullptr;

	vector<GameObject*> objects;
};
