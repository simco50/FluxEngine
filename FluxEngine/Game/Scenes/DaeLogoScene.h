#pragma once
#include "../../Scenegraph/SceneBase.h"

class DefaultMaterial;
class GameObject;
class FlexMousePicker;
class FlexDebugRenderer;
class BasicMaterial_Deferred;

class FlexSystem;

class DaeLogoScene : public SceneBase
{
public:
	DaeLogoScene();
	~DaeLogoScene();

	void Initialize();
	void Update();
	void LateUpdate() override;
	void Render();

	enum InputID
	{
		FLEX_UI
	};
private:
	unique_ptr<DefaultMaterial> m_pGroundMaterial;
	unique_ptr<DefaultMaterial> m_pDefaultMaterial;

	GameObject* m_pCollision = nullptr;

	FlexSystem* m_pFlexSystem = nullptr;
	FlexDebugRenderer* m_pFlexDebugRenderer = nullptr;
	FlexMousePicker* m_pFlexMousePicker = nullptr;

	vector<float> deltaTimes;

	bool m_FlexUpdate = false;

	float m_Nr = 0;
};

