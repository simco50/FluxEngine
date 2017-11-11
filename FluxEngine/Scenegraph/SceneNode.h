#pragma once
class Scene;
class Component;
class Transform;

class SceneNode
{
public:
	SceneNode();
	SceneNode(const std::string& name);
	~SceneNode();

	virtual void OnSceneSet(Scene* pScene);
	virtual void Update();

	void AddChild(SceneNode* pNode);
	void AddComponent(Component* pComponent);

	Transform* GetTransform() const { return m_pTransform.get();}

	SceneNode* GetParent() const { return m_pParent; }

	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }

private:
	std::string m_Name;

	Scene* m_pScene;
	std::vector<Component*> m_Components;
	SceneNode* m_pParent = nullptr;

	unique_ptr<Transform> m_pTransform;
};