#include "FluxEngine.h"
#include "SceneNode.h"
#include "Scene.h"
#include "Component.h"

SceneNode::SceneNode(Context* pContext, const std::string& name)
	: Object(pContext), m_Name(name)
{
	m_Position = Vector3(0, 0, 0);
	m_Rotation = Quaternion::Identity;
	m_Scale = Vector3(1, 1, 1);
}

SceneNode::SceneNode(Context* pContext, Scene* pScene)
	: Object(pContext),	m_pScene(pScene)
{
	m_Position = Vector3(0, 0, 0);
	m_Rotation = Quaternion::Identity;
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

Component* SceneNode::CreateComponent(StringHash typeHash)
{
	Component* pComponent = NewObject<Component>(typeHash);
	if (pComponent)
	{
		if (!AddComponent(pComponent))
		{
			delete pComponent;
		}
	}
	return pComponent;
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

bool SceneNode::AddComponent(Component* pComponent)
{
	if (GetComponent(pComponent->GetType()) != nullptr && !pComponent->CanHaveMultiple())
	{
		FLUX_LOG(Warning, "[SceneNode::AddComponent] > SceneNode already has a %s", pComponent->GetTypeName());
		return false;
	}

	m_Components.push_back(pComponent);

	//If the node is already added to the scene
	pComponent->OnNodeSet(this);
	if (m_pScene)
	{
		pComponent->OnSceneSet(m_pScene);
	}
	return true;
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

void SceneNode::GetComponentsInChildren(StringHash type, std::vector<Component*>& components) const
{
	Component* pComp = GetComponent(type);
	if (pComp)
	{
		components.push_back(pComp);
	}
	for (SceneNode* pChild : m_Children)
	{
		pChild->GetComponentsInChildren(type, components);
	}
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
	if (space == Space::World)
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
	Rotate(Quaternion::CreateFromYawPitchRoll(Math::ToRadians * y, Math::ToRadians * x, Math::ToRadians * z), space);
}

void SceneNode::Rotate(const Quaternion& quaternion, const Space space)
{
	if (space == Space::World)
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

void SceneNode::SetLocalPositionSilent(const Vector3& newPosition)
{
	m_Position = newPosition;
}

void SceneNode::SetLocalScaleSilent(const Vector3& newScale)
{
	m_Scale = newScale;
}

void SceneNode::SetLocalRotationSilent(const Quaternion& newRotation)
{
	m_Rotation = newRotation;
}

const Vector3& SceneNode::GetPosition() const
{
	return m_Position;
}

void SceneNode::SetPosition(const Vector3& newPosition, const Space space)
{
	if (space == Space::World)
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
	if (space == Space::World)
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
	SetRotation(Quaternion::CreateFromYawPitchRoll(Math::ToRadians * y, Math::ToRadians * x, Math::ToRadians * z), space);
}

void SceneNode::SetRotation(const Quaternion& quaternion, const Space space)
{
	if (space == Space::World)
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

void SceneNode::SetTransform(const Matrix& transform, Space space /*= Space::World*/)
{
	if (space == Space::World)
	{
		Matrix temp = transform;
		Vector3 wPos, wScale;
		Quaternion wRot;
		temp.Decompose(wScale, wRot, wPos);
		SetPosition(wPos, Space::World);
		SetRotation(wRot, Space::World);
		SetScale(wScale, Space::World);
	}
	else
	{
		Matrix temp = transform;
		temp.Decompose(m_Scale, m_Rotation, m_Position);
		MarkDirty();
	}
}

void SceneNode::LookInDirection(const Vector3& direction)
{
	SetRotation(Math::LookRotation(direction), Space::World);
}
