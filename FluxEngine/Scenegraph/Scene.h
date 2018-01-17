#pragma once
#include "Core\Object.h"

class SceneNode;
class Drawable;
class Graphics;
class Renderer;
class Component;

class Scene : public Object
{
	FLUX_OBJECT(Scene, Object);

public:
	Scene(Graphics* pGraphics);
	virtual ~Scene();

	virtual void Initialize();
	virtual void Update();

	void AddChild(SceneNode* pNode);
	Renderer* GetRenderer() const { return m_pRenderer.get(); }
	Camera* GetCamera() const;

	SceneNode* FindNode(const std::string& name);

	void AddComponent(Component* pComponent);

	Component* GetComponent(StringHash hash);

	template<typename T>
	T* GetComponent()
	{
		return static_cast<T*>(GetComponent(T::GetTypeStatic()));
	}

	template<typename T, typename ...Args>
	T* GetOrCreateComponent(Args ...args)
	{
		T* pComponent = GetComponent<T>();
		if (pComponent)
			return pComponent;
		pComponent = new T(args...);
		AddComponent(pComponent);
		return pComponent;
	}

private:
	unique_ptr<Renderer> m_pRenderer;
	std::vector<SceneNode*> m_pNodes;

	std::vector<Component*> m_Components;
};