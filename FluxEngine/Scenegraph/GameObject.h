#pragma once
class SceneBase;
class ComponentBase;
class TransformComponent;

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void Initialize() {}
	virtual void Update() {}
	virtual void Render() {}

	void AddComponent(ComponentBase* pComponent);

	void AddChild(GameObject* pChild);

	GameObject* GetParent() const { return m_pParent; }
	TransformComponent* GetTransform();

	std::vector<GameObject*> GetChildren() const { return m_pChildren; }

	void SetTag(const wstring& tag) { m_Tag = tag; }
	const wstring& GetTag() const { return m_Tag; }

	void SetName(const wstring& name) { m_Name = name; }
	const wstring& GetName() const { return m_Name; }

	GameObject* Find(const wstring& name);

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
		DebugLog::Log(L"GameObject::RemoveComponent() -> Object does not have this component!", LogType::WARNING);
	}

	SceneBase* GetScene();

protected:
	GameContext* m_pGameContext = nullptr;

private:
	friend class SceneBase;
	friend class Transform;
	friend class ShadowMapper;

	void BaseInitialize(GameContext* pGameContext);
	void BaseUpdate();
	void BaseRender();

	wstring m_Tag;
	wstring m_Name;
	GameObject* m_pParent = nullptr;
	SceneBase* m_pScene = nullptr;
	std::vector<GameObject*> m_pChildren;
	std::vector<ComponentBase*> m_pComponents;
	TransformComponent* m_pTransform = nullptr;

	bool m_IsInitialized = false;
};