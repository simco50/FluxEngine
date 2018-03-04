#pragma once

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

class FluxCore
{
public:
	FluxCore();
	virtual ~FluxCore();

	DELETE_COPY(FluxCore)

	int Run(HINSTANCE hInstance);
	void GameLoop();
	void RenderUI();
	void InitGame();

	static void DoExit();

private:
	void OnPause(bool isActive);

	//Window variables
	HINSTANCE m_hInstance = nullptr;

	std::unique_ptr<Scene> m_pScene;
	FreeCamera* m_pCamera = nullptr;
	SceneNode* m_pNode = nullptr;

	bool m_DebugPhysics = false;

	//Systems
	std::unique_ptr<Console> m_pConsole;
	Graphics* m_pGraphics = nullptr;
	ImmediateUI* m_pImmediateUI = nullptr;
	InputEngine* m_pInput = nullptr;
	PhysicsSystem* m_pPhysics = nullptr;
	DebugRenderer* m_pDebugRenderer = nullptr;
	ResourceManager* m_pResourceManager = nullptr;

	Context* m_pContext;

	static bool m_Exiting;
};
