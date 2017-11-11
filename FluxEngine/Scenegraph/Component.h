#pragma once
class Scene;
class SceneNode;
class Transform;

class Component
{
public:
	Component();
	virtual ~Component();

	virtual void OnSceneSet(Scene* pScene);
	virtual void OnNodeSet(SceneNode* pNode);

	virtual void OnNodeRemoved();
	virtual void OnSceneRemoved();

	Transform* GetTransform();

	virtual void Update() = 0;

protected:
	Scene* m_pScene = nullptr;
	SceneNode* m_pNode = nullptr;
};