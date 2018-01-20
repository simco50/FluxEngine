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
	void SetPosition(const Vector3& newPosition, const Space space = Space::WORLD);
	void SetPosition(const float x, const float y, const float z, const Space space = Space::WORLD);

	void SetScale(const float uniformScale);
	void SetScale(const Vector3& scale);
	void SetScale(const float x, const float y, const float z);

	void SetRotation(const Vector3& eulerAngles, const Space space = Space::WORLD);
	void SetRotation(const float x, const float y, const float z, const Space space = Space::WORLD);

	void SetRotation(const Quaternion& quaternion, const Space space = Space::WORLD);

	//Relative
	void Translate(const Vector3& translation, const Space space = Space::WORLD);
	void Translate(const float x, const float y, const float z, const Space space = Space::WORLD);

	void Rotate(const Vector3& eulerAngles, const Space space = Space::WORLD);
	void Rotate(const float x, const float y, const float z, const Space space = Space::WORLD);
	void Rotate(const Quaternion& quaternion, const Space space = Space::WORLD);

	Vector3 TransformVector(const Vector3& input, const TransformElement elements) const;

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