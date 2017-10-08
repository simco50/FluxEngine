#pragma once
class SceneBase;
class ComponentBase;
class Transform;

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void Initialize() {}
	virtual void Update() {}

	void AddComponent(ComponentBase* pComponent);

	void AddChild(GameObject* pChild);

	GameObject* GetParent() const { return m_pParent; }
	Transform* GetTransform();

	std::vector<GameObject*> GetChildren() const { return m_pChildren; }

	void SetTag(const string& tag) { m_Tag = tag; }
	const string& GetTag() const { return m_Tag; }

	void SetName(const string& name) { m_Name = name; }
	const string& GetName() const { return m_Name; }

	GameObject* Find(const string& name);

	template<class T>
	T* GetComponent()
	{
		const type_info& info = typeid(T);
		for (ComponentBase* pComp : m_pComponents)
		{
			if (info == typeid(*pComp))
				return static_cast<T*>(pComp);
		}
		return nullptr;
	}
	template<class T>
	void RemoveComponent()
	{
		const type_info& info = typeid(T);
		for (size_t i = 0; i < m_pComponents.size(); i++)
		{
			if (info == typeid(*m_pComponents[i]))
			{
				SafeDelete(m_pComponents[i]);
				m_pComponents.erase(m_pComponents.begin() + i);
				return;
			}
		}
		FLUX_LOG(WARNING, "GameObject::RemoveComponent() -> Object does not have this component!");
	}

	SceneBase* GetScene();

protected:
	GameContext* m_pGameContext = nullptr;

private:
	friend class SceneBase;
	friend class Transform;
	friend class ShadowMapper;
	friend class FluxCore;

	void BaseInitialize(GameContext* pGameContext);
	void BaseUpdate();

	string m_Tag;
	string m_Name;
	GameObject* m_pParent = nullptr;
	SceneBase* m_pScene = nullptr;
	std::vector<GameObject*> m_pChildren;
	std::vector<ComponentBase*> m_pComponents;
	Transform* m_pTransform = nullptr;

	bool m_IsInitialized = false;
};