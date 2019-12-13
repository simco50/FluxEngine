#pragma once
#include "Scenegraph\SceneNode.h"

class Camera;
class InputEngine;

class FreeObject : public SceneNode
{
	FLUX_OBJECT(FreeObject, SceneNode)

public:
	explicit FreeObject(Context* pContext);
	virtual ~FreeObject();

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	void Update();

	void UseMouseAndKeyboard(const bool use) { m_UseMouseAndKeyboard = use; }

	void SetSpeed(const float speed) { m_MoveSpeed = speed; }
private:
	InputEngine* m_pInput = nullptr;

	void KeyboardMouse();
	void Controller();

	float m_MoveSpeed = 100.0f;
	float m_ShiftMultiplier = 3.0f;
	float m_RotationSpeed = 20.0f;
	DelegateHandle m_UpdateHandle;

	bool m_UseMouseAndKeyboard = true;
};

