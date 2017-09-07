#include "stdafx.h"
#include "Transform.h"
#include "../Scenegraph/GameObject.h"

Transform::Transform()
{
	m_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMVECTOR quat = XMQuaternionIdentity();
	XMStoreFloat4(&m_Rotation, quat);
	m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

Transform::~Transform()
{
}

void Transform::Initialize()
{
	OnLocalChange();
}

void Transform::Update()
{
	if(m_PrevChanged)
	{
		m_HasChanged = false;
		m_PrevChanged = false;
	}
	else
		m_PrevChanged = m_HasChanged;
}

void Transform::OnLocalChange()
{
	XMMATRIX xmLocalTranslation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	XMVECTOR xmRotation = XMLoadFloat4(&m_Rotation);
	XMMATRIX xmLocalRotation = XMMatrixRotationQuaternion(xmRotation);
	XMMATRIX xmLocalScale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);

	XMMATRIX xmLocalMatrix = xmLocalScale * xmLocalRotation * xmLocalTranslation;
	GameObject* pParent = m_pGameObject->GetParent();

	XMMATRIX xmWorldMatrix = xmLocalMatrix;
	if (pParent != nullptr)
	{
		XMMATRIX xmParentWorldMatrix = XMLoadFloat4x4(&pParent->GetTransform()->GetWorldMatrix());
		xmWorldMatrix *= xmParentWorldMatrix;
		
		XMVECTOR xmWorldRotation, xmWorldScale, xmWorldTranslation;
		XMMatrixDecompose(&xmWorldScale, &xmWorldRotation, &xmWorldTranslation, xmWorldMatrix);

		XMStoreFloat3(&m_WorldPosition, xmWorldTranslation);
		XMStoreFloat3(&m_WorldScale, xmWorldScale);
		XMStoreFloat4(&m_WorldRotation, xmWorldRotation);
	}
	else
	{
		m_WorldPosition = m_Position;
		m_WorldRotation = m_Rotation;
		m_WorldScale = m_Scale;
	}

	XMStoreFloat4x4(&m_WorldMatrix, xmWorldMatrix);

	UpdateDirections();

	m_HasChanged = true;
}

void Transform::OnWorldChange()
{
	GameObject* pParent = m_pGameObject->GetParent();

	XMMATRIX xmWorldTranslation = XMMatrixTranslation(m_WorldPosition.x, m_WorldPosition.y, m_WorldPosition.z);
	XMVECTOR xmRotation = XMLoadFloat4(&m_WorldRotation);
	XMMATRIX xmWorldRotation = XMMatrixRotationQuaternion(xmRotation);
	XMMATRIX xmWorldScale = XMMatrixScaling(m_WorldScale.x, m_WorldScale.y, m_WorldScale.z);

	XMMATRIX xmWorld = xmWorldScale * xmWorldRotation * xmWorldTranslation;
	XMStoreFloat4x4(&m_WorldMatrix, xmWorld);

	XMMATRIX xmLocal = xmWorld;

	if(pParent != nullptr)
	{
		XMMATRIX xmParentWorld = XMLoadFloat4x4(&pParent->GetTransform()->GetWorldMatrix());
		XMMATRIX xmParentWorldInv = XMMatrixInverse(nullptr, xmParentWorld);

		xmLocal = xmLocal * xmParentWorldInv;

		XMVECTOR localScale, localRotation, localTranslation;
		XMMatrixDecompose(&localScale, &localRotation, &localTranslation, xmLocal);
		XMStoreFloat3(&m_Scale, localScale);
		XMStoreFloat3(&m_Position, localTranslation);
		XMStoreFloat4(&m_Rotation, localRotation);
	}
	else
	{
		m_Position = m_WorldPosition;
		m_Rotation = m_WorldRotation;
		m_Scale = m_WorldScale;
	}
	XMStoreFloat4x4(&m_LocalMatrix, xmLocal);

	UpdateDirections();

	m_HasChanged = true;
}

void Transform::UpdateDirections()
{
	XMVECTOR xmWorldRotation = XMLoadFloat4(&m_WorldRotation);
	XMMATRIX rotMat = XMMatrixRotationQuaternion(xmWorldRotation);
	XMVECTOR forward = XMVector3TransformCoord(XMVectorSet(0, 0, 1, 0), rotMat);
	XMVECTOR right = XMVector3TransformCoord(XMVectorSet(1, 0, 0, 0), rotMat);
	XMVECTOR up = XMVector3Cross(forward, right);

	XMStoreFloat3(&m_Forward, forward);
	XMStoreFloat3(&m_Right, right);
	XMStoreFloat3(&m_Up, up);
}

void Transform::Render()
{
}

void Transform::Translate(const XMVECTOR& translation, const Space space)
{
	XMVECTOR xmPos = XMLoadFloat3(&m_WorldPosition);
	if (space == Space::SELF)
	{
		XMMATRIX xmWorld = XMLoadFloat4x4(&m_WorldMatrix);
		xmPos = XMVector3Transform(translation, xmWorld);
		XMStoreFloat3(&m_WorldPosition, xmPos);
	}
	else
	{
		xmPos += translation;
		XMStoreFloat3(&m_WorldPosition, xmPos);
	}
	OnWorldChange();
}

void Transform::Translate(const XMFLOAT3& translation, const Space space)
{
	Translate(XMLoadFloat3(&translation), space);
}

void Transform::Translate(const float x, const float y, const float z, const Space space)
{
	Translate(XMFLOAT3(x, y, z), space);
}

void Transform::Rotate(const XMFLOAT3& eulerAngles, const Space space)
{
	Rotate(eulerAngles.x, eulerAngles.y, eulerAngles.z, space);
}

void Transform::Rotate(const float x, const float y, const float z, const Space space)
{
	XMVECTOR xmRotation = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));
	Rotate(xmRotation, space);
}

void Transform::Rotate(const XMFLOAT4& quaternion, const Space space)
{
	XMVECTOR quat = XMLoadFloat4(&quaternion);
	Rotate(quat, space);
}

void Transform::Rotate(const XMVECTOR& quaternion, const Space space)
{
	if (space == Space::WORLD)
	{
		XMVECTOR xmCurrRot = XMLoadFloat4(&m_WorldRotation);
		xmCurrRot = XMQuaternionMultiply(xmCurrRot, quaternion);
		XMStoreFloat4(&m_WorldRotation, xmCurrRot);
		OnWorldChange();
	}
	else
	{
		XMVECTOR xmCurrRot = XMLoadFloat4(&m_Rotation);
		xmCurrRot = XMQuaternionMultiply(quaternion, xmCurrRot);
		XMStoreFloat4(&m_Rotation, xmCurrRot);
		OnLocalChange();
	}
}

XMFLOAT3 Transform::TransformVector(const XMFLOAT3& input, const TransformElement elements) const
{
	XMMATRIX xmMatrix = XMMatrixIdentity();
	XMMATRIX xmWorld = XMLoadFloat4x4(&m_WorldMatrix);
	XMVECTOR xmScale, xmRot, xmTranslate;
	XMMatrixDecompose(&xmScale, &xmRot, &xmTranslate, xmWorld);
	if (elements & TransformElement::SCALE)
		xmMatrix = XMMatrixMultiply(xmMatrix, XMMatrixScalingFromVector(xmScale));
	if (elements & TransformElement::ROTATION)
		xmMatrix = XMMatrixMultiply(xmMatrix, XMMatrixRotationQuaternion(xmRot));
	if (elements & TransformElement::POSITION)
		xmMatrix = XMMatrixMultiply(xmMatrix, XMMatrixTranslationFromVector(xmTranslate));
	XMVECTOR xmInput = XMLoadFloat3(&input);
	xmInput = XMVector3Transform(xmInput, xmMatrix);
	XMFLOAT3 output;
	XMStoreFloat3(&output, xmInput);
	return output;
}

void Transform::SetPosition(const XMFLOAT3& newPosition, const Space space)
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
}

void Transform::SetPosition(const float x, const float y, const float z, const Space space)
{
	SetPosition(XMFLOAT3(x, y, z), space);
}

void Transform::SetPosition(const XMVECTOR& newPosition, const Space space)
{
	if (space == Space::WORLD)
	{
		XMStoreFloat3(&m_WorldPosition, newPosition);
		OnWorldChange();
	}
	else
	{
		XMStoreFloat3(&m_Position, newPosition);
		OnLocalChange();
	}
}

void Transform::SetScale(const XMFLOAT3& scale)
{
	m_WorldScale = scale;
	OnWorldChange();
}

void Transform::SetScale(const float x, const float y, const float z)
{
	m_WorldScale = XMFLOAT3(x, y, z);
	OnWorldChange();
}

void Transform::SetRotation(const XMFLOAT3& eulerAngles, const Space space)
{
	SetRotation(eulerAngles.x, eulerAngles.y, eulerAngles.z, space);
}

void Transform::SetRotation(const float x, const float y, const float z, const Space space)
{
	XMVECTOR xmRotation = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z));
	if (space == Space::WORLD)
	{
		XMStoreFloat4(&m_WorldRotation, xmRotation);
		OnWorldChange();
	}
	else
	{
		XMStoreFloat4(&m_Rotation, xmRotation);
		OnLocalChange();
	}
}

void Transform::SetRotation(const XMVECTOR& quaternion, const Space space)
{
	if(space == Space::WORLD)
	{
		XMStoreFloat4(&m_WorldRotation, quaternion);
		OnWorldChange();
	}
	else
	{
		XMStoreFloat4(&m_Rotation, quaternion);
		OnLocalChange();
	}
}

void Transform::SetRotation(const XMFLOAT4& quaternion, const Space space)
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
}
