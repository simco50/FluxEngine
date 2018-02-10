#pragma once
class Scene;
class Component;
class Transform;

class SceneNode : public Object
{
	FLUX_OBJECT(SceneNode, Object)

public:
	SceneNode(Context* pContext, const std::string& name = "");
	virtual ~SceneNode();

	virtual void OnSceneSet(Scene* pScene);
	virtual void Update();

	void AddChild(SceneNode* pNode);
	void AddComponent(Component* pComponent);

	Transform* GetTransform() const { return m_pTransform.get();}
	SceneNode* GetParent() const { return m_pParent; }

	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }

	template<class T>
	T* GetComponent()
	{
		return static_cast<T*>(GetComponent(T::GetTypeStatic()));
	}

	Component* GetComponent(StringHash type);

	template<typename T, typename ...Args>
	T* GetOrCreateComponent(Args ...args)
	{
		T* pComponent = GetComponent<T>();
		if (pComponent)
			return pComponent;
		pComponent = new T(m_pContext, args...);
		AddComponent(pComponent);
		return pComponent;
	}

	void OnTransformDirty(const Transform* pTransform);

protected:
	//Constructor used for Scene to be able to initialize with "this"
	SceneNode(Context* pContext, Scene* pScene);

	unique_ptr<Transform> m_pTransform;
	std::vector<Component*> m_Components;

	SceneNode* m_pParent = nullptr;
	Scene* m_pScene = nullptr;

private:
	std::string m_Name;
};