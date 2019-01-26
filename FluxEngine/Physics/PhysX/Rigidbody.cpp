#include "FluxEngine.h"
#include "Rigidbody.h"
#include "PhysicsSystem.h"
#include "PhysicsScene.h"
#include "Scenegraph\SceneNode.h"
#include "Scenegraph\Scene.h"

using namespace physx;

Rigidbody::Rigidbody(Context* pContext) :
	Component(pContext)
{
	m_pPhysicsSystem = pContext->GetSubsystem<PhysicsSystem>();
}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);

	m_pPhysicsScene = pScene->GetOrCreateComponent<PhysicsScene>();
	m_pPhysicsScene->AddRigidbody(this);
}

void Rigidbody::OnSceneRemoved()
{
	Component::OnSceneRemoved();

	if (m_pPhysicsScene && m_pBody)
	{
		m_pPhysicsScene->RemoveRigidbody(this);
		m_pBody = nullptr;
	}
}

void Rigidbody::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);

	CreateBody(m_Type);
}

void Rigidbody::OnNodeRemoved()
{
	Component::OnNodeRemoved();

	if (m_pPhysicsScene && m_pBody)
	{
		m_pPhysicsScene->GetScene()->removeActor(*m_pBody);
		m_pBody = nullptr;
	}
}

void Rigidbody::OnMarkedDirty(const SceneNode* pNode)
{
	Vector3 wPos = pNode->GetWorldPosition();
	Quaternion wRot = pNode->GetWorldRotation();

	PxTransform transform(*reinterpret_cast<PxVec3*>(&wPos), *reinterpret_cast<PxQuat*>(&wRot));

	if (m_pBody)
		m_pBody->setGlobalPose(transform, true);
}

void Rigidbody::UpdateBody()
{
	if (m_pBody)
	{
		PxTransform transform = m_pBody->getGlobalPose();
		m_pNode->SetPosition(*reinterpret_cast<Vector3*>(&transform.p), Space::World);
		m_pNode->SetRotation(*reinterpret_cast<Quaternion*>(&transform.q), Space::World);
	}
}

void Rigidbody::SetKinematic(const bool isKinematic)
{
	if (m_pBody && m_Type == Type::Dynamic)
		reinterpret_cast<PxRigidDynamic*>(m_pBody)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, isKinematic);
}

void Rigidbody::SetBodyType(const Type type)
{
	if (m_Type == type)
	{
		return;
	}
	CreateBody(type);
}

void Rigidbody::SetConstraints(const Constraint constraints)
{
	if (constraints == m_Constraints)
		return;
	m_Constraints = constraints;

	if(m_pConstraintJoint == nullptr && m_pBody)
		m_pConstraintJoint = PxD6JointCreate(*m_pPhysicsSystem->GetPhysics(), nullptr, m_pBody->getGlobalPose(), m_pBody, PxTransform(PxIdentity));

	if (m_pConstraintJoint)
	{
		if (constraints == Constraint::None)
		{
			m_pConstraintJoint->release();
			m_pConstraintJoint = nullptr;
		}
		else
		{
			m_pConstraintJoint->setMotion(PxD6Axis::eX, (constraints & Constraint::XPosition) ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
			m_pConstraintJoint->setMotion(PxD6Axis::eY, (constraints & Constraint::YPosition) ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
			m_pConstraintJoint->setMotion(PxD6Axis::eZ, (constraints & Constraint::ZPosition) ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);

			m_pConstraintJoint->setMotion(PxD6Axis::eTWIST, (constraints & Constraint::XRotation) ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
			m_pConstraintJoint->setMotion(PxD6Axis::eSWING1, (constraints & Constraint::YRotation) ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
			m_pConstraintJoint->setMotion(PxD6Axis::eSWING2, (constraints & Constraint::ZPosition) ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);

			m_pConstraintJoint->setLocalPose(PxJointActorIndex::eACTOR0, m_pBody->getGlobalPose());
		}
	}
}

void Rigidbody::CreateBody(const Type type)
{
	m_Type = type;
	if (m_pNode == nullptr)
	{
		return;
	}

	AUTOPROFILE(Rigidbody_CreateBody);

	Vector3 wPos = m_pNode->GetWorldPosition();
	Quaternion wRot = m_pNode->GetWorldRotation();
	PxTransform transform(*reinterpret_cast<PxVec3*>(&wPos), *reinterpret_cast<PxQuat*>(&wRot));

	PxRigidActor* pNewBody;
	switch (m_Type)
	{
	case Rigidbody::Type::Dynamic:
		pNewBody = m_pPhysicsSystem->GetPhysics()->createRigidDynamic(transform);
		break;
	case Rigidbody::Type::Static:
	default:
		pNewBody = m_pPhysicsSystem->GetPhysics()->createRigidStatic(transform);
		break;
	}

	if (m_pBody)
	{
		//Remove all the shapes from the old body and attach to the new one
		std::vector<PxShape*> shapes(m_pBody->getNbShapes());
		m_pBody->getShapes(shapes.data(), (PxU32)shapes.size(), 0);

		for (PxShape* pShape : shapes)
		{
			m_pBody->detachShape(*pShape);
			pNewBody->attachShape(*pShape);
		}
		if (m_pScene)
		{
			m_pPhysicsScene->GetScene()->removeActor(*m_pBody);
		}
	}
	m_pBody = pNewBody;
	m_pBody->userData = this;
	if (m_pScene)
	{
		m_pPhysicsScene->GetScene()->addActor(*m_pBody);
	}
}

void Rigidbody::CreateUI()
{
	static const char* types[] = {
		"Static",
		"Dynamic",
		"Kinematic",
	};
	int current = (int)m_Type;
	ImGui::Combo("Type", &current, [](void* data, int index, const char** pOut) 
	{
		const char** pText = (const char**)data;
		*pOut = pText[index];
		return true; 
	}, &types, sizeof(types) / sizeof(types[0]));
	if (current != (int)m_Type)
	{
		SetBodyType((Type)current);
	}
}