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
	virtual void OnSceneRemoved();

	template<typename T, typename ...Args>
	T* CreateComponent(Args ...args)
	{
		T* pComponent = new T(m_pContext, args...);
		AddComponent(pComponent);
		return pComponent;
	}

	SceneNode* CreateChild(const std::string& name = "");

	template<typename T>
	T* CreateChild(const std::string& name = "")
	{
		T* pChild = new T(m_pContext);
		AddChild(pChild);
		pChild->SetName(name);
		return pChild;
	}

	Transform* GetTransform() const { return m_pTransform.get();}
	SceneNode* GetParent() const { return m_pParent; }

	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetName() const { return m_Name; }

	template<class T>
	T* GetComponent()
	{
		return static_cast<T*>(GetComponent(T::GetTypeStatic()));
	}

	Component* GetComponent(StringHash type) const;
	const std::vector<Component*>& GetComponents() const { return m_Components; }
	const std::vector<SceneNode*>& GetChildren() const { return m_Children; }

	template<typename T, typename ...Args>
	T* GetOrCreateComponent(Args ...args)
	{
		T* pComponent = GetComponent<T>();
		if (pComponent)
		{
			return pComponent;
		}
		pComponent = new T(m_pContext, args...);
		AddComponent(pComponent);
		return pComponent;
	}

	void OnTransformDirty(const Transform* pTransform);

protected:
	//Constructor used for Scene to be able to initialize with "this"
	SceneNode(Context* pContext, Scene* pScene);
	void AddChild(SceneNode* pNode);

	std::unique_ptr<Transform> m_pTransform;
	std::vector<Component*> m_Components;
	std::vector<SceneNode*> m_Children;

	SceneNode* m_pParent = nullptr;
	Scene* m_pScene = nullptr;

private:
	void AddComponent(Component* pComponent);
	std::string m_Name;
};