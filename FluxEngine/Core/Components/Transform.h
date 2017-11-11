#pragma once

class SceneNode;

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

class Transform
{
public:
	Transform(SceneNode* pNode);
	~Transform();

	void Initialize();
	void Update();

	//Absolute
	void SetPosition(const XMFLOAT3& newPosition, const Space space = Space::WORLD);
	void SetPosition(const float x, const float y, const float z, const Space space = Space::WORLD);
	void SetPosition(const XMVECTOR& newPosition, const Space space = Space::WORLD);

	void SetScale(const float uniformScale);
	void SetScale(const XMFLOAT3& scale);
	void SetScale(const float x, const float y, const float z);

	void SetRotation(const XMFLOAT3& eulerAngles, const Space space = Space::WORLD);
	void SetRotation(const float x, const float y, const float z, const Space space = Space::WORLD);

	void SetRotation(const XMVECTOR& quaternion, const Space space = Space::WORLD);
	void SetRotation(const XMFLOAT4& quaternion, const Space space = Space::WORLD);

	//Relative
	void Translate(const XMVECTOR& translation, const Space space = Space::WORLD);
	void Translate(const XMFLOAT3& translation, const Space space = Space::WORLD);
	void Translate(const float x, const float y, const float z, const Space space = Space::WORLD);

	void Rotate(const XMFLOAT3& eulerAngles, const Space space = Space::WORLD);
	void Rotate(const float x, const float y, const float z, const Space space = Space::WORLD);
	void Rotate(const XMFLOAT4& quaternion, const Space space = Space::WORLD);
	void Rotate(const XMVECTOR& quaternion, const Space space = Space::WORLD);

	XMFLOAT3 TransformVector(const XMFLOAT3& input, const TransformElement elements) const;

	const XMFLOAT3& GetPosition() const { return m_Position; }
	const XMFLOAT3& GetWorldPosition() const { return m_WorldPosition; }

	const XMFLOAT3& GetScale() const { return m_Scale; }
	const XMFLOAT3& GetWorldScale() const { return m_WorldScale; }

	const XMFLOAT4& GetRotation() const { return m_Rotation; }
	const XMFLOAT4& GetWorldRotation() const { return m_WorldRotation; }

	const XMFLOAT4X4& GetWorldMatrix() const { return m_WorldMatrix; }

	const XMFLOAT3& GetUp() const { return m_Up; }
	const XMFLOAT3& GetForward() const { return m_Forward; }
	const XMFLOAT3& GetRight() const { return m_Right; }

	void OnLocalChange();
	void OnWorldChange();
	void UpdateDirections();

	bool HasChanged() const { return m_HasChanged; }

private:
	SceneNode* m_pNode;

	XMFLOAT3 m_Up;
	XMFLOAT3 m_Right;
	XMFLOAT3 m_Forward;

	XMFLOAT4X4 m_WorldMatrix;
	XMFLOAT3 m_WorldPosition;
	XMFLOAT4 m_WorldRotation;
	XMFLOAT3 m_WorldScale;

	XMFLOAT4X4 m_LocalMatrix;
	XMFLOAT4 m_Rotation;
	XMFLOAT3 m_Position;
	XMFLOAT3 m_Scale;

	bool m_PrevChanged = false;
	bool m_HasChanged = false;
};