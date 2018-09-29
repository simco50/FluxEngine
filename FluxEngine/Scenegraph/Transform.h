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

	//Absolute
	void SetPosition(const Vector3& newPosition, Space space = Space::WORLD);
	void SetPosition(float x, float y, float z, Space space = Space::WORLD);

	void SetScale(float uniformScale);
	void SetScale(const Vector3& scale);
	void SetScale(float x, float y, float z);

	void SetRotation(const Vector3& eulerAngles, Space space = Space::WORLD);
	void SetRotation(float x, float y, float z, Space space = Space::WORLD);

	void SetRotation(const Quaternion& quaternion, Space space = Space::WORLD);

	//Relative
	void Translate(const Vector3& translation, Space space = Space::WORLD);
	void Translate(float x, float y, float z, Space space = Space::WORLD);

	void Rotate(const Vector3& eulerAngles, Space space = Space::WORLD);
	void Rotate(float x, float y, float z, Space space = Space::WORLD);
	void Rotate(const Quaternion& quaternion, Space space = Space::WORLD);

	Vector3 TransformVector(const Vector3& input, TransformElement elements) const;

	const Vector3& GetPosition() const { return m_Position; }
	const Vector3& GetWorldPosition() const { return m_WorldPosition; }

	const Vector3& GetScale() const { return m_Scale; }
	const Vector3& GetWorldScale() const { return m_WorldScale; }

	const Quaternion& GetRotation() const { return m_Rotation; }
	const Quaternion& GetWorldRotation() const { return m_WorldRotation; }

	const Matrix& GetWorldMatrix() const { return m_WorldMatrix; }

	const Vector3& GetUp() const { return m_Up; }
	const Vector3& GetForward() const { return m_Forward; }
	const Vector3& GetRight() const { return m_Right; }

	void OnLocalChange();
	void OnWorldChange();
	void UpdateDirections();

	//Mark the transform as dirty so it will recalculate the transforms
	void MarkDirty(const Vector3& position, const Vector3& scale, const Quaternion& rotation);

private:
	SceneNode* m_pNode;

	Vector3 m_Up;
	Vector3 m_Right;
	Vector3 m_Forward;

	Matrix m_WorldMatrix;
	Vector3 m_WorldPosition;
	Quaternion m_WorldRotation;
	Vector3 m_WorldScale;

	Quaternion m_Rotation;
	Vector3 m_Position;
	Vector3 m_Scale;
};