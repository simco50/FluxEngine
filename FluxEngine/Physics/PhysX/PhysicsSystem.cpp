#include "FluxEngine.h"
#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
{
	AUTOPROFILE(PhysicsSystem_CreatePhysics);

	m_pFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, m_AllocatorCallback, m_ErrorCallback);
	if (m_pFoundation == nullptr)
		FLUX_LOG(ERROR, "[PhysxSystem::Initialize()] > Failed to create PxFoundation");

	m_pPvd = PxCreatePvd(*m_pFoundation);
	if (m_pPvd == nullptr)
		FLUX_LOG(ERROR, "[PhysxSystem::Initialize()] > Failed to create PxPvd");
	m_pPvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_pPvd->connect(*m_pPvdTransport, PxPvdInstrumentationFlag::eALL);

	bool recordMemoryAllocations = true;
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), recordMemoryAllocations, m_pPvd);
	if (m_pPhysics == nullptr)
		FLUX_LOG(ERROR, "[PhysxSystem::Initialize()] > Failed to create PxPhysics");

	if (!PxInitExtensions(*m_pPhysics, m_pPvd))
		FLUX_LOG(ERROR, "PxInitExtensions failed!");

	m_pCpuDispatcher = PxDefaultCpuDispatcherCreate(3);

	m_pDefaultMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.5f);
}

PhysicsSystem::~PhysicsSystem()
{
	PxCloseExtensions();
	((PxDefaultCpuDispatcher*)(m_pCpuDispatcher))->release();
	m_pCpuDispatcher = nullptr;
	m_pPhysics->release();
	m_pPhysics = nullptr;
	m_pPvd->release();
	m_pPvd = nullptr;
	m_pPvdTransport->release();
	m_pPvdTransport = nullptr;
	m_pFoundation->release();
	m_pFoundation = nullptr;
}

physx::PxFilterFlags PhysicsSystem::SimulationFilterShader(PxFilterObjectAttributes attribute0, PxFilterData filterData0, PxFilterObjectAttributes attribute1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	UNREFERENCED_PARAMETER(constantBlock);
	UNREFERENCED_PARAMETER(constantBlockSize);

	// let triggers through
	if (PxFilterObjectIsTrigger(attribute0) || PxFilterObjectIsTrigger(attribute1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}
	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	// trigger the contact callback for pairs (A,B) where
	// the filtermask of A contains the ID of B and vice versa.
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

	return PxFilterFlag::eDEFAULT;
}
