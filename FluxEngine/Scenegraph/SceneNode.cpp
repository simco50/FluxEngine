#include "FluxEngine.h"
#include "SceneNode.h"
#include "Scene.h"
#include "Component.h"

SceneNode::SceneNode(Context* pContext, const std::string& name)
	: Object(pContext), m_Name(name)
{
	m_Position = Vector3(0, 0, 0);
	m_Rotation = XMQuaternionIdentity();
	m_Scale = Vector3(1, 1, 1);
}

SceneNode::SceneNode(Context* pContext, Scene* pScene)
	: Object(pContext),	m_pScene(pScene)
{
	m_Position = Vector3(0, 0, 0);
	m_Rotation = XMQuaternionIdentity();
	m_Scale = Vector3(1, 1, 1);
}

SceneNode::~SceneNode()
{
	for (Component*& pComponent : m_Components)
	{
		delete pComponent;
	}
	m_Components.clear();
}

void SceneNode::OnSceneSet(Scene* pScene)
{
	m_pScene = pScene;

	//The component don't have the scene assigned yet
	for (Component* pComponent : m_Components)
	{
		pComponent->OnSceneSet(pScene);
	}
}

void SceneNode::OnSceneRemoved()
{
	m_pScene = nullptr;

	for (Component* pComponent : m_Components)
	{
		pComponent->OnSceneRemoved();
	}
}

SceneNode* SceneNode::CreateChild(const std::string& name)
{
	SceneNode* pNode = new SceneNode(m_pContext, name);
	AddChild(pNode);
	return pNode;
}

void SceneNode::AddChild(SceneNode* pNode)
{
	m_Children.push_back(pNode);
	pNode->m_pParent = this;
	pNode->OnSceneSet(m_pScene);
	m_pScene->TrackChild(pNode);
}

void SceneNode::AddComponent(Component* pComponent)
{
	if (GetComponent(pComponent->GetType()) != nullptr && !pComponent->CanHaveMultiple())
	{
		FLUX_LOG(Error, "[SceneNode::AddComponent] > SceneNode already has a %s", pComponent->GetTypeName().c_str());
		return;
	}

	m_Components.push_back(pComponent);

	//If the node is already added to the scene
	pComponent->OnNodeSet(this);
	if (m_pScene)
	{
		pComponent->OnSceneSet(m_pScene);
	}
}

Component* SceneNode::GetComponent(StringHash type) const
{
	for (Component* pComponent : m_Components)
	{
		if (pComponent->IsTypeOf(type))
		{
			return pComponent;
		}
	}
	return nullptr;
}

Vector3 SceneNode::GetWorldPosition() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return GetWorldMatrix().Translation();
}

const Vector3& SceneNode::GetScale() const
{
	return m_Scale;
}

Vector3 SceneNode::GetWorldScale() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return Math::ScaleFromMatrix(m_WorldMatrix);
}

const Quaternion& SceneNode::GetRotation() const
{
	return m_Rotation;
}

const Quaternion& SceneNode::GetWorldRotation() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return m_WorldRotation;
}

const Matrix& SceneNode::GetWorldMatrix() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return m_WorldMatrix;
}

Vector3 SceneNode::GetUp() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return XMVector3Rotate(Vector3(0, 1, 0), m_WorldRotation);
}

Vector3 SceneNode::GetForward() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return XMVector3Rotate(Vector3(0, 0, 1), m_WorldRotation);
}

Vector3 SceneNode::GetRight() const
{
	if (m_Dirty)
	{
		UpdateWorld();
	}
	return XMVector3Rotate(Vector3(1, 0, 0), m_WorldRotation);
}

void SceneNode::MarkDirty()
{
	m_Dirty = true;
	for (Component* pComponent : m_Components)
	{
		pComponent->OnMarkedDirty(this);
	}
	for (SceneNode* pChild : m_Children)
	{
		pChild->MarkDirty();
	}
}

void SceneNode::UpdateWorld() const
{
	Matrix localTranslation = Matrix::CreateTranslation(m_Position);
	Matrix localScale = Matrix::CreateScale(m_Scale);
	Matrix localRotation = Matrix::CreateFromQuaternion(m_Rotation);
	m_WorldMatrix = localScale * localRotation * localTranslation;
	m_WorldRotation = m_Rotation;

	if (m_pParent && m_pParent != m_pScene)
	{
		m_WorldMatrix *= m_pParent->GetWorldMatrix();
		m_WorldRotation *= m_pParent->GetWorldRotation();
	}

	m_Dirty = false;
}

void SceneNode::Translate(const Vector3& translation, const Space space)
{
	if (space == Space::WORLD)
	{
		if (m_pParent && m_pParent != m_pScene)
		{
			m_Position += XMVector3Transform(translation, XMMatrixInverse(nullptr, m_pParent->GetWorldMatrix()));
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

void SceneNode::Translate(const float x, const float y, const float z, const Space space)
{
	Translate(Vector3(x, y, z), space);
}

void SceneNode::Rotate(const Vector3& eulerAngles, const Space space)
{
	Rotate(eulerAngles.x, eulerAngles.y, eulerAngles.z, space);
}

void SceneNode::Rotate(const float x, const float y, const float z, const Space space)
{
	Rotate(Quaternion::CreateFromYawPitchRoll(Math::DegToRad(y), Math::DegToRad(x), Math::DegToRad(z)), space);
}

void SceneNode::Rotate(const Quaternion& quaternion, const Space space)
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

const Vector3& SceneNode::GetPosition() const
{
	return m_Position;
}

void SceneNode::SetPosition(const Vector3& newPosition, const Space space)
{
	if (space == Space::WORLD)
	{
		if (m_pParent && m_pParent != m_pScene)
		{
			m_Position = XMVector3Transform(newPosition, XMMatrixInverse(nullptr, m_pParent->GetWorldMatrix()));
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

void SceneNode::SetPosition(const float x, const float y, const float z, const Space space)
{
	SetPosition(Vector3(x, y, z), space);
}

void SceneNode::SetScale(const Vector3& scale, Space space)
{
	if (space == Space::WORLD)
	{
		if (m_pParent && m_pParent != m_pScene)
		{
			m_Scale = scale / m_pParent->GetWorldScale();
		}
		else
		{
			m_Scale = scale;
		}
	}
	else
	{
		m_Scale = scale;
	}
	MarkDirty();
}

void SceneNode::SetScale(const float x, const float y, const float z, Space space)
{
	SetScale(Vector3(x, y, z), space);
}

void SceneNode::SetScale(const float uniformScale, Space space)
{
	SetScale(Vector3(uniformScale, uniformScale, uniformScale), space);
}

void SceneNode::SetRotation(const Vector3& eulerAngles, const Space space)
{
	SetRotation(eulerAngles.x, eulerAngles.y, eulerAngles.z, space);
}

void SceneNode::SetRotation(const float x, const float y, const float z, const Space space)
{
	SetRotation(Quaternion::CreateFromYawPitchRoll(Math::DegToRad(y), Math::DegToRad(x), Math::DegToRad(z)), space);
}

void SceneNode::SetRotation(const Quaternion& quaternion, const Space space)
{
	if (space == Space::WORLD)
	{
		if (m_pParent && m_pParent != m_pScene)
		{
			m_Rotation = XMQuaternionMultiply(quaternion, XMQuaternionInverse(m_pParent->GetWorldRotation()));
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

void SceneNode::SetTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale, Space space /*= Space::WORLD*/)
{
	SetPosition(position, space);
	SetRotation(rotation, space);
	SetScale(scale, space);
}

void SceneNode::LookInDirection(const Vector3& direction)
{
	Vector3 v;
	direction.Normalize(v);
	float pitch = asin(-v.y);
	float yaw = atan2(v.x, v.z);
	SetRotation(Math::RadToDeg(pitch), Math::RadToDeg(yaw), 0, Space::WORLD);
}
