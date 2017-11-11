#pragma once
class Scene;
class Component;
class Transform;

class SceneNode
{
public:
	SceneNode();
	~SceneNode();

	virtual void OnSceneSet(Scene* pScene);
	virtual void Update();

	void AddChild(SceneNode* pNode);
	void AddComponent(Component* pComponent);

	Transform* GetTransform() const { return m_pTransform.get();}

	SceneNode* GetParent() const { return m_pParent; }

private:
	Scene* m_pScene;
	std::vector<Component*> m_Components;
	SceneNode* m_pParent = nullptr;

	unique_ptr<Transform> m_pTransform;
};