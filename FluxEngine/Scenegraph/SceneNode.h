#pragma once
class Scene;
class Component;
class Transform;

class SceneNode
{
public:
	SceneNode();
	SceneNode(const std::string& name);
	virtual ~SceneNode();

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

	SceneNode* m_pParent = nullptr;
	Scene* m_pScene = nullptr;

	unique_ptr<Transform> m_pTransform;
	std::vector<Component*> m_Components;
};