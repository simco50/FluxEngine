#pragma once
#include "../Scenegraph/SceneBase.h"

class DefaultMaterial;
class GameObject;
class FlexMousePicker;
class FlexDebugRenderer;
class BasicMaterial_Deferred;
class UI_Canvas;

class FlexSystem;

enum InputID
{
	FLEX_UI
};

class SoftBodyScene : public SceneBase
{
public:
	SoftBodyScene();
	~SoftBodyScene();

	void Initialize();
	void Update();
	void LateUpdate();
	void Render();
private:
	unique_ptr<DefaultMaterial> m_pGroundMaterial;
	unique_ptr<DefaultMaterial> m_pDefaultMaterial;

	GameObject* m_pCollision = nullptr;

	FlexSystem* m_pFlexSystem = nullptr;
	FlexDebugRenderer* m_pFlexDebugRenderer = nullptr;
	FlexMousePicker* m_pFlexMousePicker = nullptr;

	bool m_FlexUpdate = false;

	UI_Canvas* m_pCanvas = nullptr;
	float m_Nr = 0;
};

