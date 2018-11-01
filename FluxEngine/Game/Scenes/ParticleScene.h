#pragma once
#include "../../Scenegraph/SceneBase.h"

class DefaultMaterial;
class GameObject;
class FlexMousePicker;
class FlexDebugRenderer;
class BasicMaterial_Deferred;
class ParticleEmitterComponent;

class FluidRenderer;

struct FlexSolver;

class FlexSystem;

class ParticleScene : public SceneBase
{
public:
	ParticleScene();
	~ParticleScene();

	void Initialize();
	void Update();
	void LateUpdate();
	void Render();

	enum InputID
	{
		RESTART,
		FLEX_DEBUG,
		FLEX_SIMULATE,
		FLEX_TELEPORT,
	};
private:
	unique_ptr<DefaultMaterial> m_pGroundMaterial;
	FlexSystem* m_pFlexSystem = nullptr;
	FlexDebugRenderer* m_pFlexDebugRenderer = nullptr;
	FlexMousePicker* m_pFlexMousePicker = nullptr;
	bool m_FlexUpdate = false;

	ParticleEmitterComponent* m_pEmitter = nullptr;
};

