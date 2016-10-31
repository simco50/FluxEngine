#pragma once
#include "../Scenegraph/SceneBase.h"
#include "../Physics/Flex/FlexHelper.h"

class DefaultMaterial;
class GameObject;
class FlexMousePicker;
class FlexDebugRenderer;
class BasicMaterial_Deferred;

class FlexSystem;

enum InputID
{
	RESTART,
	FLEX_DEBUG,
	FLEX_SIMULATE,
	FLEX_TELEPORT,
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
};

