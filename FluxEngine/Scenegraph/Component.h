#pragma once
class Scene;
class SceneNode;
class Transform;

class Component
{
public:
	Component() {}
	virtual ~Component() {}

	virtual void OnSceneSet(Scene* pScene)
	{
		if (m_pScene)
			return;
		m_pScene = pScene;
	}
	virtual void OnNodeSet(SceneNode* pNode)
	{
		if (m_pNode)
			return;
		m_pNode = pNode;
	}
	Transform* GetTransform();

	virtual void Update() = 0;

protected:
	Scene* m_pScene = nullptr;
	SceneNode* m_pNode = nullptr;
};