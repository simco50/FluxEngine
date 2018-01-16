#include "FluxEngine.h"
#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
{

}

PhysicsSystem::~PhysicsSystem()
{

}

void PhysicsSystem::Initialize()
{
	m_pFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, m_AllocatorCallback, m_ErrorCallback);
	if(m_pFoundation == nullptr)
		FLUX_LOG(ERROR, "[PhysxSystem::Initialize()] > Failed to create PxFoundation");

	m_pPvd = PxCreatePvd(*m_pFoundation);
	if(m_pPvd == nullptr)
		FLUX_LOG(ERROR, "[PhysxSystem::Initialize()] > Failed to create PxPvd");
	m_pPvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_pPvd->connect(*m_pPvdTransport, PxPvdInstrumentationFlag::eALL);

	bool recordMemoryAllocations = true;
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), recordMemoryAllocations, m_pPvd);
	if (m_pPhysics == nullptr)
		FLUX_LOG(ERROR, "[PhysxSystem::Initialize()] > Failed to create PxPhysics");
}

void PhysicsSystem::SetupPvdConnection()
{

}

void PhysicsSystem::Shutdown()
{
	m_pPhysics->release();
	m_pPhysics = nullptr;
	m_pPvd->release();
	m_pPvd = nullptr;
	m_pPvdTransport->release();
	m_pPvdTransport = nullptr;
	m_pFoundation->release();
	m_pFoundation = nullptr;
}
