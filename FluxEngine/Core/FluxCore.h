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
class PostProcessing;
class Model;

class FluxCore : public Object
{
public:
	FluxCore(Context* pContext);
	virtual ~FluxCore();

	DELETE_COPY(FluxCore)

	int Run(HINSTANCE hInstance);
	void ProcessFrame();
	void InitGame();

	static void DoExit();
	static bool IsExiting() { return m_Exiting; }

private:
	void RenderUI();

	//Window variables
	HINSTANCE m_hInstance = nullptr;

	std::unique_ptr<Scene> m_pScene;
	FreeCamera* m_pCamera = nullptr;

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
	PostProcessing* m_pPostProcessing = nullptr;

	Model* m_pModel = nullptr;

	SceneNode* m_pSelectedNode = nullptr;
	static bool m_Exiting;
};
