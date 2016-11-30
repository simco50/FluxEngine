#pragma once
#include "../Scenegraph/SceneBase.h"

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
	FlexSystem* m_pSystem = nullptr;
	FluidRenderer* m_pFluidRenderer = nullptr;

	ParticleEmitterComponent* m_pEmitter = nullptr;
};

