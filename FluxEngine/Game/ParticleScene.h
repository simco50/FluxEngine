#pragma once
#include "../Scenegraph/SceneBase.h"

class DefaultMaterial;
class GameObject;
class FlexMousePicker;
class FlexDebugRenderer;
class BasicMaterial_Deferred;

class FlexFluidRenderer;

struct FlexSolver;

class FlexSystem;

enum InputID
{
	RESTART,
	FLEX_DEBUG,
	FLEX_SIMULATE,
	FLEX_TELEPORT,
};

class ParticleScene : public SceneBase
{
public:
	ParticleScene();
	~ParticleScene();

	void Initialize();
	void Update();
	void LateUpdate();
	void Render();
private:
	unique_ptr<DefaultMaterial> m_pGroundMaterial;
	FlexSystem* m_pSystem = nullptr;
	FlexFluidRenderer* m_pFluidRenderer = nullptr;
};

