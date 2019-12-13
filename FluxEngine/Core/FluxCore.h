#pragma once
#include "Object.h"

class Graphics;
class ImmediateUI;
class InputEngine;
class FreeCamera;
class Scene;
class SceneNode;
class PhysicsSystem;
class DebugRenderer;
class Context;
class ResourceManager;
class AudioEngine;
class Material;
class FreeObject;

class FluxCore : public Object
{
	FLUX_OBJECT(FluxCore, Object)

public:
	explicit FluxCore(Context* pContext);
	virtual ~FluxCore();

	DELETE_COPY(FluxCore)

	int Run(HINSTANCE hInstance);
	void ProcessFrame();
	void InitGame();

	static void DoExit();
	static bool IsExiting() { return m_Exiting; }

private:
	void RenderUI();
	void GameUpdate();
	void ObjectUI(SceneNode* pNode);

	void ComponentUI(StringHash type);

	std::unique_ptr<Scene> m_pScene;
	FreeCamera* m_pCamera = nullptr;

	bool m_EnableDebugRendering = true;
	bool m_DebugPhysics = false;

	//Systems
	std::unique_ptr<Console> m_pConsole;
	Graphics* m_pGraphics = nullptr;
	ImmediateUI* m_pImmediateUI = nullptr;
	InputEngine* m_pInput = nullptr;
	PhysicsSystem* m_pPhysics = nullptr;
	DebugRenderer* m_pDebugRenderer = nullptr;
	ResourceManager* m_pResourceManager = nullptr;
	AudioEngine* m_pAudioEngine = nullptr;

	Material* m_pManMaterial = nullptr;
	SceneNode* m_pFrame = nullptr;
	FreeObject* m_pVirtualCamera = nullptr;

	SceneNode* m_pSelectedNode = nullptr;
	static bool m_Exiting;
	int m_FramesToCapture = 1;

	float m_VirtualFoV = 60.0f;
	float m_CameraDistance = 500;
};
