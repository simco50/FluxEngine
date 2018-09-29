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

	const Vector3& GetPosition() const;
	Vector3 GetWorldPosition() const;

	const Vector3& GetScale() const;
	const Vector3& GetWorldScale() const;

	const Quaternion& GetRotation() const;
	const Quaternion& GetWorldRotation() const;

	const Matrix& GetWorldMatrix() const;

	Vector3 GetUp() const;
	Vector3 GetForward() const;
	Vector3 GetRight() const;

	//Mark the transform as dirty so it will recalculate the transforms
	void MarkDirty();

private:
	void UpdateWorld() const;

	SceneNode* m_pNode;

	mutable Matrix m_WorldMatrix;
	mutable Quaternion m_WorldRotation;

	Quaternion m_Rotation;
	Vector3 m_Position;
	Vector3 m_Scale;

	mutable bool m_Dirty = true;
};