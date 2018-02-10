#pragma once

class Graphics;
class ImmediateUI;
class InputEngine;
class FreeCamera;
class Scene;
class SceneNode;
class PhysicsSystem;
class Window;
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

private:
	void OnPause(bool isActive);

	//Window variables
	HINSTANCE m_hInstance = nullptr;

	std::unique_ptr<Scene> m_pScene;
	FreeCamera* m_pCamera = nullptr;
	SceneNode* m_pNode = nullptr;

	bool m_DebugPhysics = false;

	//Systems
	Graphics* m_pGraphics = nullptr;
	ImmediateUI* m_pImmediateUI = nullptr;
	InputEngine* m_pInput = nullptr;
	PhysicsSystem* m_pPhysics = nullptr;
	DebugRenderer* m_pDebugRenderer = nullptr;
	ResourceManager* m_pResourceManager = nullptr;

	Window* m_pWindow;
	Context* m_pContext;
};
