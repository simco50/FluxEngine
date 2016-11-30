#pragma once
#include "../Core/FluxCore.h"
#include "../Helpers/Singleton.h"
class SceneBase;

class GameManager :	public FluxCore, public Singleton<GameManager>
{
public:
	GameManager();
	~GameManager();
	
	void LoadScene(SceneBase* pScene);
	void LoadSceneClean(SceneBase* pScene);

private:
	void PrepareGame();
	void Initialize(EngineContext* pEngineContext);
	void Render();
	void Update();
	void OnResize();
	EngineContext* m_pEngineContext = nullptr;
	SceneBase* m_pActiveScene = nullptr;
	SceneBase* m_pOldScene = nullptr;

	bool m_SceneInitialized = false;

	vector<SceneBase*> m_pScenes;
};