#pragma once
#include "../Scenegraph/SceneBase.h"
#include "../Physics/Flex/FlexHelper.h"

struct FlexSolver;
struct FlexParams;
class DefaultMaterial;
class GameObject;
class FlexMousePicker;
class FlexDebugRenderer;

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
	void SetFlexData();

	static const int FLEX_MAX_PARTICLES = 50000;
	static const int FLEX_SUBSTEPS = 3;
	const FlexMemory m_MemoryType = eFlexMemoryHost;

	unique_ptr<DefaultMaterial> m_pGroundMaterial;
	FlexSolver* m_pFlexSolver = nullptr;
	FlexHelper::FlexData m_FlexData;

	FlexDebugRenderer* m_pFlexDebugRenderer = nullptr;
	FlexMousePicker* m_pFlexMousePicker = nullptr;

	bool transed = false;

	bool m_FlexUpdate = false;
};

