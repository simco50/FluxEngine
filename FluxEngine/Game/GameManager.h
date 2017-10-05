#pragma once
#include "../Core/FluxCore.h"
class SceneBase;

class GameManager :	public FluxCore
{
public:
	GameManager();
	~GameManager();
	
	void AddScene(SceneBase* pScene);
	void LoadScene(const int index);
	void ReloadScene(SceneBase* pScene);

	int GetCurrentScene() const { return m_ActiveScene; }

private:
	void PrepareGame();
	void Initialize();
	void Update();

	int m_ActiveScene = -1;
	bool m_SceneInitialized = false;
	vector<SceneBase*> m_pScenes;
};