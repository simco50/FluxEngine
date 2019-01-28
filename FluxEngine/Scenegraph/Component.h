#pragma once
class Scene;
class SceneNode;

class Component : public Object
{
	FLUX_OBJECT(Component, Object)

public:
	explicit Component(Context* pContext);
	virtual ~Component();

	virtual void OnSceneSet(Scene* pScene);
	virtual void OnNodeSet(SceneNode* pNode);

	virtual void OnNodeRemoved();
	virtual void OnSceneRemoved();
	//Broadcast transform change to all components
	virtual void OnMarkedDirty(const SceneNode* /*pNode*/) { }

	virtual bool CanHaveMultiple() const { return false; }

	SceneNode* GetNode() const { return m_pNode; }

	Component* GetComponent(StringHash type) const;
	template<typename T>
	T* GetComponent() const
	{
		return static_cast<T*>(GetComponent(T::GetTypeStatic()));
	}

	virtual void CreateUI() {}

	void SetEnabled(const bool enabled) { m_Enabled = enabled; }
	bool IsEnabled() const { return m_Enabled; }

protected:
	bool m_Enabled = true;

	Scene* m_pScene = nullptr;
	SceneNode* m_pNode = nullptr;
};