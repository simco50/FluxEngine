#pragma once
class GameObject;
class DeferredRenderer;
class ShadowMapper;

class SceneBase
{
public:

	SceneBase();
	virtual ~SceneBase();

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void LateUpdate() {}

	void AddChild(GameObject* pChild);
	void OnResize();

	SceneContext* GetSceneContext() const { return m_pGameContext->Scene; }

protected:
	GameContext* m_pGameContext = nullptr;
	std::vector<GameObject*> m_pChildren;

	GameObject* FindObject(const wstring& name);

private:
	friend class GameManager;
	bool m_Paused = false;
	GameContext m_GameContext;
	SceneContext m_SceneContext;

	void BaseInitialize(EngineContext* pEngineContext);
	void BaseUpdate();
	void BaseRender();
	DeferredRenderer* m_pDeferredRenderer = nullptr;
};