#pragma once
class Scene;
class SceneNode;
class Transform;

#ifdef _DEBUG
#define REQUIRE_COMPONENT(T) \
std::stringstream str; \
str << GetTypeName() << " requires " << T::GetTypeNameStatic(); \
std::string err = str.str(); \
checkf(GetComponent<T>() != nullptr, err.c_str())
#else
#define REQUIRE_COMPONENT(T)
#endif

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

	Transform* GetTransform();

	Component* GetComponent(StringHash type);
	template<typename T>
	T* GetComponent()
	{
		return static_cast<T*>(GetComponent(T::GetTypeStatic()));
	}

	virtual void Update() = 0;

protected:
	Scene* m_pScene = nullptr;
	SceneNode* m_pNode = nullptr;
};