#include "FluxEngine.h"
#include "PhysicsScene.h"
#include "PhysicsSystem.h"
#include "Scenegraph\Scene.h"
#include "Rendering\Camera\Camera.h"
#include "Rigidbody.h"
#include "Collider.h"

using namespace physx;

const float PhysicsScene::FIXED_TIME_STEP = 0.02f;

PhysicsScene::PhysicsScene(Context* pContext) :
	Component(pContext)
{
	m_pPhysicsSystem = pContext->GetSubsystem<PhysicsSystem>();
}

PhysicsScene::~PhysicsScene()
{
}

void PhysicsScene::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);
	pScene->OnSceneUpdate().AddRaw(this, &PhysicsScene::Update);

	PxCudaContextManager* pCudaContextManager = m_pPhysicsSystem->GetCudaContextManager();

	AUTOPROFILE(PhysicsScene_CreateScene);
	PxSceneDesc desc(m_pPhysicsSystem->GetPhysics()->getTolerancesScale());
	desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	desc.cpuDispatcher = m_pPhysicsSystem->GetCpuDispatcher();
	desc.filterShader = PhysicsSystem::SimulationFilterShader;
	desc.simulationEventCallback = this;
	desc.userData = this;
	if (pCudaContextManager)
	{
		desc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
		desc.broadPhaseType = PxBroadPhaseType::eGPU;
		desc.gpuDispatcher = pCudaContextManager->getGpuDispatcher();
	}

	m_pPhysicsScene = m_pPhysicsSystem->GetPhysics()->createScene(desc);
	m_pPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	m_pPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	m_pPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
	m_pPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
	m_pPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1.0f);
}

void PhysicsScene::OnSceneRemoved()
{
	Component::OnSceneRemoved();

	PhysXSafeRelease(m_pPhysicsScene);
}

void PhysicsScene::Update()
{
	AUTOPROFILE(PhysicsScene_Update);

	float frameTime = GameTimer::DeltaTime();
	if (frameTime > 0.25f)
		frameTime = 0.25f;
	m_TimeAccumulator += frameTime;
	while (m_TimeAccumulator >= FIXED_TIME_STEP)
	{
		m_pPhysicsScene->simulate(FIXED_TIME_STEP);
		m_pPhysicsScene->fetchResults(true);
		m_TimeAccumulator -= FIXED_TIME_STEP;
	}

	//Send camera location to Pvd
#ifdef NDEBUG
	/*Transform* pTransform = m_pScene->GetCamera()->GetTransform();
	m_pPhysicsScene->getScenePvdClient()->updateCamera(
		"Main Camera",
		*reinterpret_cast<const PxVec3*>(&pTransform->GetWorldPosition()),
		*reinterpret_cast<const PxVec3*>(&pTransform->GetUp()),
		*reinterpret_cast<const PxVec3*>(&pTransform->GetForward()));*/
#endif

	for (Rigidbody* pRigidbody : m_Rigidbodies)
		pRigidbody->UpdateBody();
}

bool PhysicsScene::Raycast(const Ray& ray, RaycastResult& outResult, const float length) const
{
	outResult = RaycastResult();

	if (m_pPhysicsScene == nullptr)
		return false;

	PxRaycastBuffer buffer;
	bool hit = m_pPhysicsScene->raycast(
		*reinterpret_cast<const PxVec3*>(&ray.position),
		*reinterpret_cast<const PxVec3*>(&ray.direction),
		length,
		buffer,
		PxHitFlag::eDEFAULT,
		PxQueryFilterData()
	);
	if (!hit)
		return false;

	outResult.pCollider = static_cast<Collider*>(buffer.block.shape->userData);
	outResult.pRigidbody = static_cast<Rigidbody*>(buffer.block.actor->userData);
	outResult.Hit = buffer.hasBlock;
	outResult.Normal = *reinterpret_cast<Vector3*>(&buffer.block.normal);
	outResult.Position = *reinterpret_cast<Vector3*>(&buffer.block.position);
	return true;
}

void PhysicsScene::SetGravity(const float x, const float y, const float z)
{
	if(m_pPhysicsScene)
		m_pPhysicsScene->setGravity(PxVec3(x, y, z));
}

void PhysicsScene::AddRigidbody(Rigidbody* pRigidbody)
{
	if (pRigidbody)
	{
		m_Rigidbodies.push_back(pRigidbody);
		m_pPhysicsScene->addActor(*pRigidbody->GetBody());
	}
}

void PhysicsScene::RemoveRigidbody(Rigidbody* pRigidbody)
{
	if (pRigidbody)
	{
		m_Rigidbodies.erase(std::remove(m_Rigidbodies.begin(), m_Rigidbodies.end(), pRigidbody), m_Rigidbodies.end());
		m_pPhysicsScene->removeActor(*pRigidbody->GetBody());
	}
}

void PhysicsScene::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	for (PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER)
			continue;

		Collider* pShapeA = reinterpret_cast<Collider*>(pairs[i].triggerShape->userData);
		Collider* pShapeB = reinterpret_cast<Collider*>(pairs[i].otherShape->userData);
		if (pShapeA)
		{
			Rigidbody* pBody = reinterpret_cast<Rigidbody*>(pairs[i].triggerActor->userData);
			if (pBody)
			{
				if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					pBody->OnTriggerEnter().Broadcast(pShapeB);
				else if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					pBody->OnTriggerExit().Broadcast(pShapeB);
			}
		}
	}
}

void PhysicsScene::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	for (PxU32 i = 0; i < nbPairs; ++i)
	{
		Collider* pShapeA = reinterpret_cast<Collider*>(pairs[i].shapes[0]->userData);
		Collider* pShapeB = reinterpret_cast<Collider*>(pairs[i].shapes[1]->userData);
		if (pShapeA)
		{
			Rigidbody* pBody = reinterpret_cast<Rigidbody*>(pairHeader.actors[0]->userData);
			if (pBody)
			{
				CollisionResult result;
				std::vector<PxContactPairPoint> contactPoints(pairs[i].contactCount);
				pairs[i].extractContacts(contactPoints.data(), pairs[i].contactStreamSize);
				for (const PxContactPairPoint& contactPoint : contactPoints)
				{
					CollisionResult::Contact contact;
					contact.Position = *reinterpret_cast<const Vector3*>(&contactPoint.position);
					contact.Normal = *reinterpret_cast<const Vector3*>(&contactPoint.normal);
					result.Contacts.push_back(contact);
				}
				result.pCollider = pShapeB;
				result.pRigidbody = reinterpret_cast<Rigidbody*>(pairHeader.actors[1]->userData);
				result.pNode = pShapeB->GetNode();

				if (pairs[i].events.isSet(PxPairFlag::eNOTIFY_TOUCH_FOUND))
				{
					pBody->OnCollisionEnter().Broadcast(result);
				}
				else if (pairs[i].events.isSet(PxPairFlag::eNOTIFY_TOUCH_LOST))
				{
					pBody->OnCollisionExit().Broadcast(result);
				}
			}
		}
	}
}
