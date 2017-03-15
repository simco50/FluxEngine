#include "stdafx.h"
#include "SimulationShaderFilter.h"
#include "PhysicsCore.h"

PhysicsCore::PhysicsCore()
{
}

PhysicsCore::~PhysicsCore()
{
}

void PhysicsCore::Initialize()
{
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_AllocatorCallback, m_ErrorCallback);

	m_pProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(m_pFoundation);
	if (!m_pProfileZoneManager)
		Console::Log("Hello", LogType::ERROR);

	bool recordMemoryAllocations = true;
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), recordMemoryAllocations, m_pProfileZoneManager);
	if (!m_pPhysics)
		Console::Log("Hello", LogType::ERROR);

	m_pDefaultCpuDispatcher = PxDefaultCpuDispatcherCreate(1);

	PxSceneDesc sceneDesc = PxSceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_pDefaultCpuDispatcher;
	sceneDesc.gpuDispatcher = nullptr;
	sceneDesc.filterShader = SimulationFilter::SimulationFilterShader;
	m_pScene = m_pPhysics->createScene(sceneDesc);

	// check if PvdConnection manager is available on this platform
	if (m_pPhysics->getPvdConnectionManager() == NULL)
		return;

	// setup connection parameters
	const char*     pvd_host_ip		= "127.0.0.1";  // IP of the PC which is running PVD
	int             port			= 5425;			// TCP port to connect to, where PVD is listening
	unsigned int    timeout			= 100;          // timeout in milliseconds to wait for PVD to respond, consoles and remote PCs need a higher timeout.
	PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();

	// and now try to connect
	m_pPvdConnection = PxVisualDebuggerExt::createConnection(m_pPhysics->getPvdConnectionManager(), pvd_host_ip, port, timeout, connectionFlags);
}

void PhysicsCore::Update()
{
	m_pScene->simulate(GameTimer::DeltaTime());
	m_pScene->fetchResults(true);
}

void PhysicsCore::Finalize()
{
	m_pScene->release();
	m_pPhysics->release();
	if (m_pPvdConnection)
		m_pPvdConnection->release();
	m_pDefaultCpuDispatcher->release();
	m_pProfileZoneManager->release();
	m_pFoundation->release();
}
