#pragma once
class Scene;
class SceneNode;
class Transform;

class Component : public Object
{
	FLUX_OBJECT(Component, Object)

public:
	Component();
	virtual ~Component();

	virtual void OnSceneSet(Scene* pScene);
	virtual void OnNodeSet(SceneNode* pNode);

	virtual void OnNodeRemoved();
	virtual void OnSceneRemoved();
	//Broadcast transform change to all components
	virtual void OnMarkedDirty(const Transform* transform) { UNREFERENCED_PARAMETER(transform); }
	virtual void Update() {}

	virtual bool CanHaveMultiple() const { return false; }

	Transform* GetTransform();
	SceneNode* GetNode() { return m_pNode; }

	Component* GetComponent(StringHash type);
	template<typename T>
	T* GetComponent()
	{
		return static_cast<T*>(GetComponent(T::GetTypeStatic()));
	}

protected:
	Scene* m_pScene = nullptr;
	SceneNode* m_pNode = nullptr;
};