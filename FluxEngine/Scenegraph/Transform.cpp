#include "FluxEngine.h"
#include "Transform.h"
#include "Scenegraph/SceneNode.h"

Transform::Transform(SceneNode *pNode) :
	m_pNode(pNode)
{
	m_Scale = Vector3(1.0f, 1.0f, 1.0f);
	m_Rotation = Quaternion::CreateFromYawPitchRoll(0, 0, 0);
	m_Position = Vector3(0.0f, 0.0f, 0.0f);
}

Transform::~Transform()
{
}

void Transform::Initialize()
{
}

Vector3 Transform::GetWorldPosition() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return GetWorldMatrix().Translation();
}

const Vector3& Transform::GetScale() const
{
	return m_Scale;
}

const Vector3& Transform::GetWorldScale() const
{
	return m_Scale;
}

const Quaternion& Transform::GetRotation() const
{
	return m_Rotation;
}

const Quaternion& Transform::GetWorldRotation() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return m_WorldRotation;
}

const Matrix& Transform::GetWorldMatrix() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return m_WorldMatrix;
}

Vector3 Transform::GetUp() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return XMVector3Rotate(Vector3(0, 1, 0), m_WorldRotation);
}

Vector3 Transform::GetForward() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return XMVector3Rotate(Vector3(0, 0, 1), m_WorldRotation);
}

Vector3 Transform::GetRight() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return XMVector3Rotate(Vector3(1, 0, 0), m_WorldRotation);
}

void Transform::MarkDirty()
{
	if (m_Dirty == false)
	{
		m_Dirty = true;
		m_pNode->OnTransformDirty(this);
	}
}

void Transform::UpdateWorld() const
{
	Matrix localTranslation = Matrix::CreateTranslation(m_Position);
	Matrix localScale = Matrix::CreateScale(m_Scale);
	Matrix localRotation = Matrix::CreateFromQuaternion(m_Rotation);
	m_WorldMatrix = localScale * localRotation * localTranslation;
	m_WorldRotation = m_Rotation;

	SceneNode* pParent = m_pNode->GetParent();
	if (pParent)
	{
		m_WorldMatrix *= pParent->GetTransform()->GetWorldMatrix();
		m_WorldRotation *= pParent->GetTransform()->GetWorldRotation();
	}

	m_Dirty = false;
}

void Transform::Translate(const Vector3& translation, const Space space)
{
	if (space == Space::WORLD)
	{
		if (m_pNode->GetParent())
		{
			m_Position += XMVector3Transform(translation, XMMatrixInverse(nullptr, m_pNode->GetParent()->GetTransform()->GetWorldMatrix()));
		}
		else
		{
			m_Position += translation;
		}
	}
	else
	{
		m_Position += XMVector3Rotate(translation, m_Rotation);
	}
	MarkDirty();
}

void Transform::Translate(const float x, const float y, const float z, const Space space)
{
	Translate(Vector3(x, y, z), space);
}

void Transform::Rotate(const Vector3& eulerAngles, const Space space)
{
	Rotate(eulerAngles.x, eulerAngles.y, eulerAngles.z, space);
}

void Transform::Rotate(const float x, const float y, const float z, const Space space)
{
	Rotate(Quaternion::CreateFromYawPitchRoll(Math::DegToRad(y), Math::DegToRad(x), Math::DegToRad(z)), space);
}

void Transform::Rotate(const Quaternion& quaternion, const Space space)
{
	if (space == Space::WORLD)
	{
		Quaternion worldRotation = GetWorldRotation();
		m_Rotation = XMQuaternionMultiply(worldRotation, XMQuaternionMultiply(XMQuaternionMultiply(quaternion, XMQuaternionInverse(worldRotation)), m_Rotation));
	}
	else
	{
		m_Rotation = XMQuaternionMultiply(quaternion, m_Rotation);
	}
	MarkDirty();
}

const Vector3& Transform::GetPosition() const
{
	return m_Position;
}

void Transform::SetPosition(const Vector3& newPosition, const Space space)
{
	if (space == Space::WORLD)
	{
		if (m_pNode->GetParent())
		{
			m_Position = XMVector3Transform(newPosition, XMMatrixInverse(nullptr, m_pNode->GetParent()->GetTransform()->GetWorldMatrix()));
		}
		else
		{
			m_Position = newPosition;
		}
	}
	else
	{
		m_Position = newPosition;
	}
	MarkDirty();
}

void Transform::SetPosition(const float x, const float y, const float z, const Space space)
{
	SetPosition(Vector3(x, y, z), space);
}

void Transform::SetScale(const Vector3& scale)
{
	m_Scale = scale;
	MarkDirty();
}

void Transform::SetScale(const float x, const float y, const float z)
{
	SetScale(Vector3(x, y, z));
}

void Transform::SetScale(const float uniformScale)
{
	SetScale(Vector3(uniformScale, uniformScale, uniformScale));
}

void Transform::SetRotation(const Vector3& eulerAngles, const Space space)
{
	SetRotation(eulerAngles.x, eulerAngles.y, eulerAngles.z, space);
}

void Transform::SetRotation(const float x, const float y, const float z, const Space space)
{
	SetRotation(Quaternion::CreateFromYawPitchRoll(Math::DegToRad(y), Math::DegToRad(x), Math::DegToRad(z)), space);
}

void Transform::SetRotation(const Quaternion& quaternion, const Space space)
{
	if (space == Space::WORLD)
	{
		if (m_pNode->GetParent())
		{
			m_Rotation = XMQuaternionMultiply(quaternion, XMQuaternionInverse(m_pNode->GetParent()->GetTransform()->GetWorldRotation()));
		}
		else
		{
			m_Rotation = quaternion;
		}
	}
	else
	{
		m_Rotation = quaternion;
	}
	MarkDirty();
}
