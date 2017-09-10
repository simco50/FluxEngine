#pragma once
#include "../Core/FluxCore.h"
#include "../Helpers/Singleton.h"
class SceneBase;

class GameManager :	public FluxCore, public Singleton<GameManager>
{
public:
	GameManager();
	~GameManager();
	
	void AddScene(SceneBase* pScene);
	void LoadScene(const int index);
	void ReloadScene(SceneBase* pScene);

	int GetCurrentScene() const { return m_ActiveScene; }

protected:
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void PrepareGame();
	void Initialize(EngineContext* pEngineContext);
	void Update();

	EngineContext* m_pEngineContext = nullptr;
	int m_ActiveScene = -1;

	bool m_SceneInitialized = false;

	vector<SceneBase*> m_pScenes;
};