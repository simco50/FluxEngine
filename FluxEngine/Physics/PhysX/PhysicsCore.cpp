#include <stdafx.h>
#include "PhysicsCore.h"
#include "../../Graphics/MeshFilter.h"

PxMaterial* PhysicsCore::DefaultMaterial = nullptr;

PhysicsCore::PhysicsCore()
{
}

PhysicsCore::~PhysicsCore()
{
}

void PhysicsCore::Initialize()
{
	//Foundation
	m_pFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, m_AllocatorCallback, m_ErrorCallback);
	if(m_pFoundation == nullptr)
		Console::Log("PhysicsCore::Initialize() > Failed to create Foundation", LogType::ERROR);
	//Pvd Connection
	m_pPvdConnection = PxCreatePvd(*m_pFoundation);
	m_pPvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10000);
	if(m_pPvdConnection->connect(*m_pPvdTransport, PxPvdInstrumentationFlag::eALL) == false)
		Console::Log("PhysicsCore::Initialize() > PVD connection failed", LogType::WARNING);


	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), true, m_pPvdConnection);
	if (!m_pPhysics)
		Console::Log("PhysicsCore::Initialize() > Failed to create Physics", LogType::ERROR);

	m_pDefaultCpuDispatcher = PxDefaultCpuDispatcherCreate(4);

	DefaultMaterial = m_pPhysics->createMaterial(1.0f, 1.0f, 0.5f);

	//Create the scene
	PxSceneDesc sceneDesc = PxSceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_pDefaultCpuDispatcher;
	sceneDesc.gpuDispatcher = nullptr;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	m_pScene = m_pPhysics->createScene(sceneDesc);

	MeshFilter* pMeshFilter = ResourceManager::Load<MeshFilter>("./Resources/Meshes/Player.flux");
	MeshFilter::VertexData pData = pMeshFilter->GetVertexData("CONVEXMESH");

	auto input = PxDefaultMemoryInputData(reinterpret_cast<PxU8*>(pData.pData), pData.Count);
	PxRigidStatic* pRb = PxCreateStatic(*m_pPhysics, PxTransform(PxVec3(0,1.5f,0)), PxConvexMeshGeometry(m_pPhysics->createConvexMesh(input)), *DefaultMaterial);
	m_pScene->addActor(*pRb);

	PxPvdSceneClient* pPvdSceneClient = m_pScene->getScenePvdClient();
	if (pPvdSceneClient)
	{
		pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
	}
	m_pScene->simulate(0.016f);
}

void PhysicsCore::Update()
{
	if(m_pScene->fetchResults(false))
	{
		m_pScene->simulate(GameTimer::DeltaTime());
	}
}

void PhysicsCore::Finalize()
{
	m_pScene->fetchResults(true);

	if (DefaultMaterial)
		DefaultMaterial->release();
	if(m_pScene)
		m_pScene->release();
	if (m_pDefaultCpuDispatcher)
		m_pDefaultCpuDispatcher->release();
	if (m_pPhysics)
		m_pPhysics->release();
	if (m_pPvdConnection)
		m_pPvdConnection->release();
	if (m_pPvdTransport)
		m_pPvdTransport->release();
	if (m_pFoundation)
		m_pFoundation->release();
}
