#pragma once
class Scene;
class Component;

enum class Space
{
	SELF,
	WORLD
};

enum TransformElement
{
	SCALE = 1 << 0,
	ROTATION = 1 << 1,
	POSITION = 1 << 2
};
DEFINE_ENUM_FLAG_OPERATORS(TransformElement)

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

	//Absolute
	void SetPosition(const Vector3& newPosition, Space space = Space::WORLD);
	void SetPosition(float x, float y, float z, Space space = Space::WORLD);

	void SetScale(float uniformScale, Space space = Space::WORLD);
	void SetScale(const Vector3& scale, Space space = Space::WORLD);
	void SetScale(float x, float y, float z, Space space = Space::WORLD);

	void SetRotation(const Vector3& eulerAngles, Space space = Space::WORLD);
	void SetRotation(float x, float y, float z, Space space = Space::WORLD);
	void SetRotation(const Quaternion& quaternion, Space space = Space::WORLD);
	void SetTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale, Space space = Space::WORLD);

	void LookInDirection(const Vector3& direction);

	//Relative
	void Translate(const Vector3& translation, Space space = Space::WORLD);
	void Translate(float x, float y, float z, Space space = Space::WORLD);

	void Rotate(const Vector3& eulerAngles, Space space = Space::WORLD);
	void Rotate(float x, float y, float z, Space space = Space::WORLD);
	void Rotate(const Quaternion& quaternion, Space space = Space::WORLD);

	const Vector3& GetPosition() const;
	Vector3 GetWorldPosition() const;

	const Vector3& GetScale() const;
	Vector3 GetWorldScale() const;

	const Quaternion& GetRotation() const;
	const Quaternion& GetWorldRotation() const;

	const Matrix& GetWorldMatrix() const;

	Vector3 GetUp() const;
	Vector3 GetForward() const;
	Vector3 GetRight() const;

	//Mark the transform as dirty so it will recalculate the transforms
	void MarkDirty();

protected:
	//Constructor used for Scene to be able to initialize with "this"
	SceneNode(Context* pContext, Scene* pScene);
	void AddChild(SceneNode* pNode);

	std::vector<Component*> m_Components;
	std::vector<SceneNode*> m_Children;

	SceneNode* m_pParent = nullptr;
	Scene* m_pScene = nullptr;

private:
	void AddComponent(Component* pComponent);
	std::string m_Name;

	void UpdateWorld() const;

	mutable Matrix m_WorldMatrix;
	mutable Quaternion m_WorldRotation;

	Quaternion m_Rotation;
	Vector3 m_Position;
	Vector3 m_Scale;

	mutable bool m_Dirty = true;
};