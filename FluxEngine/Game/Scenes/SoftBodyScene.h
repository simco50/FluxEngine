#pragma once
#include "../Scenegraph/SceneBase.h"

class DefaultMaterial;
class GameObject;
class FlexMousePicker;
class FlexDebugRenderer;
class BasicMaterial_Deferred;

class FlexSystem;

class SoftBodyScene : public SceneBase
{
public:
	SoftBodyScene();
	~SoftBodyScene();

	void Initialize();
	void Update();
	void LateUpdate();
	void Render();

	enum InputID
	{
		FLEX_UI
	};
private:
	enum MaterialID : int
	{
		PATRICK_HOUSE,
		SPONGEBOB_HOUSE,
		SQUIDWARD_HOUSE
	};

	void InitializeMaterials();

	unique_ptr<DefaultMaterial> m_pGroundMaterial;
	unique_ptr<DefaultMaterial> m_pDefaultMaterial;

	GameObject* m_pCollision = nullptr;

	FlexSystem* m_pFlexSystem = nullptr;
	FlexDebugRenderer* m_pFlexDebugRenderer = nullptr;
	FlexMousePicker* m_pFlexMousePicker = nullptr;

	vector<DefaultMaterial*> m_pMaterials;

	vector<float> deltaTimes;

	bool m_FlexUpdate = false;

	float m_Nr = 0;
};

