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
	OnLocalChange();
}

void Transform::OnLocalChange()
{
	Matrix localTranslation = Matrix::CreateTranslation(m_Position);
	Matrix localRotation = Matrix::CreateFromQuaternion(m_Rotation);
	Matrix localScale = Matrix::CreateScale(m_Scale);
	Matrix localMatrix = localScale * localRotation * localTranslation;

	SceneNode* pParent = m_pNode->GetParent();

	if (pParent != nullptr)
	{
		m_WorldMatrix = localMatrix * pParent->GetTransform()->GetWorldMatrix();
		m_WorldMatrix.Decompose(m_WorldScale, m_WorldRotation, m_WorldPosition);
	}
	else
	{
		m_WorldPosition = m_Position;
		m_WorldRotation = m_Rotation;
		m_WorldScale = m_Scale;
		m_WorldMatrix = localMatrix;
	}

	UpdateDirections();
}

void Transform::OnWorldChange()
{
	SceneNode* pParent = m_pNode->GetParent();

	Matrix worldTranslation = Matrix::CreateTranslation(m_WorldPosition);
	Matrix worldRotation = Matrix::CreateFromQuaternion(m_WorldRotation);
	Matrix worldScale = Matrix::CreateScale(m_WorldScale);
	m_WorldMatrix = worldScale * worldRotation * worldTranslation;

	if(pParent != nullptr)
	{
		Matrix parentInverse;
		pParent->GetTransform()->GetWorldMatrix().Invert(parentInverse);

		Matrix localMatrix = m_WorldMatrix * parentInverse;
		localMatrix.Decompose(m_Scale, m_Rotation, m_Position);
	}
	else
	{
		m_Position = m_WorldPosition;
		m_Rotation = m_WorldRotation;
		m_Scale = m_WorldScale;
	}
	UpdateDirections();
}

void Transform::UpdateDirections()
{
	Matrix rotMatrix = Matrix::CreateFromQuaternion(m_WorldRotation);
	m_Forward = Vector3::Transform(Vector3(0, 0, 1), rotMatrix);
	m_Right = Vector3::Transform(Vector3(1, 0, 0), rotMatrix);
	m_Up = m_Forward.Cross(m_Right);
}

void Transform::MarkDirty(const Vector3& position, const Vector3& scale, const Quaternion& rotation)
{
	m_WorldPosition = position;
	m_WorldScale = scale;
	m_WorldRotation = rotation;
	OnWorldChange();
}

void Transform::Translate(const Vector3& translation, const Space space)
{
	if (space == Space::SELF)
	{
		m_WorldPosition = Vector3::Transform(translation, m_WorldMatrix);
	}
	else
	{
		m_WorldPosition += translation;
	}
	OnWorldChange();
	m_pNode->OnTransformDirty(this);
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
	Rotate(Quaternion::CreateFromYawPitchRoll(DegToRad(y), DegToRad(x), DegToRad(z)), space);
}

void Transform::Rotate(const Quaternion& quaternion, const Space space)
{
	if (space == Space::WORLD)
	{
		m_WorldRotation *= quaternion;
		OnWorldChange();
	}
	else
	{
		m_Rotation = quaternion * m_Rotation;
		OnLocalChange();
	}
	m_pNode->OnTransformDirty(this);
}

Vector3 Transform::TransformVector(const Vector3& input, const TransformElement elements) const
{
	Matrix resultMatrix = XMMatrixIdentity();
	if (elements & TransformElement::SCALE)
		resultMatrix *= Matrix::CreateScale(m_WorldScale);
	if (elements & TransformElement::ROTATION)
		resultMatrix *= Matrix::CreateFromQuaternion(m_WorldRotation);
	if (elements & TransformElement::POSITION)
		resultMatrix *= Matrix::CreateTranslation(m_WorldPosition);
	Vector3 output;
	Vector3::Transform(input, resultMatrix, output);
	return output;
}

void Transform::SetPosition(const Vector3& newPosition, const Space space)
{
	if (space == Space::WORLD)
	{
		m_WorldPosition = newPosition;
		OnWorldChange();
	}
	else
	{
		m_Position = newPosition;
		OnLocalChange();
	}
	m_pNode->OnTransformDirty(this);
}

void Transform::SetPosition(const float x, const float y, const float z, const Space space)
{
	SetPosition(Vector3(x, y, z), space);
}

void Transform::SetScale(const Vector3& scale)
{
	m_WorldScale = scale;
	OnWorldChange();
	m_pNode->OnTransformDirty(this);
}

void Transform::SetScale(const float x, const float y, const float z)
{
	SetScale(Vector3(x, y, z));
	OnWorldChange();
}

void Transform::SetScale(const float uniformScale)
{
	SetScale(Vector3(uniformScale, uniformScale, uniformScale));
	OnWorldChange();
}

void Transform::SetRotation(const Vector3& eulerAngles, const Space space)
{
	SetRotation(eulerAngles.x, eulerAngles.y, eulerAngles.z, space);
}

void Transform::SetRotation(const float x, const float y, const float z, const Space space)
{
	if (space == Space::WORLD)
	{
		m_WorldRotation = Quaternion::CreateFromYawPitchRoll(DegToRad(y), DegToRad(x), DegToRad(z));
		OnWorldChange();
	}
	else
	{
		m_Rotation = Quaternion::CreateFromYawPitchRoll(DegToRad(y), DegToRad(x), DegToRad(z));
		OnLocalChange();
	}
	m_pNode->OnTransformDirty(this);
}

void Transform::SetRotation(const Quaternion& quaternion, const Space space)
{
	if (space == Space::WORLD)
	{
		m_WorldRotation = quaternion;
		OnWorldChange();
	}
	else
	{
		m_Rotation = quaternion;
		OnLocalChange();
	}
	m_pNode->OnTransformDirty(this);
}
