#include "FluxEngine.h"
#include "PhysicsSystem.h"
#include "Rendering\Core\Graphics.h"

#ifdef GRAPHICS_D3D11
#include "Rendering\Core\D3D11\D3D11GraphicsImpl.h"
#endif

using namespace physx;

PhysicsSystem::PhysicsSystem(Context* pContext) :
	Subsystem(pContext)
{
	AUTOPROFILE(PhysicsSystem_CreatePhysics);

	m_pFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, m_AllocatorCallback, m_ErrorCallback);
	if (m_pFoundation == nullptr)
	{
		FLUX_LOG(Error, "[PhysxSystem::Initialize()] > Failed to create PxFoundation");
	}

#ifdef _DEBUG
	m_pPvd = PxCreatePvd(*m_pFoundation);
	if (m_pPvd == nullptr)
	{
		FLUX_LOG(Error, "[PhysxSystem::Initialize()] > Failed to create PxPvd");
	}
	m_pPvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 25565, 10);
	if (!m_pPvd->connect(*m_pPvdTransport, PxPvdInstrumentationFlag::eDEBUG))
	{
		FLUX_LOG(Info, "[PhysicsSystem::PhysicsSystem] > Failed to connect to PhysX Visual Debugger");
	}
#endif

#ifdef _DEBUG
	bool recordMemoryAllocations = true;
#else
	bool recordMemoryAllocations = false;
#endif

	PxTolerancesScale scale;
	scale.length = 100;
	scale.mass = 1000;
	scale.speed = 981;
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), recordMemoryAllocations, m_pPvd);
	if (m_pPhysics == nullptr)
	{
		FLUX_LOG(Error, "[PhysxSystem::Initialize()] > Failed to create PxPhysics");
	}

	if (!PxInitExtensions(*m_pPhysics, m_pPvd))
	{
		FLUX_LOG(Error, "PxInitExtensions failed!");
	}

	m_pCpuDispatcher = PxDefaultCpuDispatcherCreate(Misc::GetCoreCount() - 1);

	m_pDefaultMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.5f);
}

PhysicsSystem::~PhysicsSystem()
{
	PxCloseExtensions();
	PxDefaultCpuDispatcher* pCpuDispatcher = (PxDefaultCpuDispatcher*)m_pCpuDispatcher;
	PhysXSafeRelease(pCpuDispatcher);
	PhysXSafeRelease(m_pPhysics);
	PhysXSafeRelease(m_pCudaContextManager);
	PhysXSafeRelease(m_pPhysics);
	PhysXSafeRelease(m_pPvd);
	PhysXSafeRelease(m_pPvdTransport);
	PhysXSafeRelease(m_pFoundation);
}

bool PhysicsSystem::InitializeCuda()
{
	AUTOPROFILE(PhysicsSystem_InitializeCuda);

	Graphics* pGraphics = GetSubsystem<Graphics>();
	if (pGraphics == nullptr)
	{
		FLUX_LOG(Error, "[PhysicsSystem::InitializeCuda] Graphics is null so can not initialize Cuda");
		return false;
	}

#ifdef GRAPHICS_D3D11
	PxCudaContextManagerDesc desc = {};
	desc.ctx = nullptr;
	desc.appGUID = "";
	desc.graphicsDevice = pGraphics->GetImpl()->GetDevice();
	desc.interopMode = PxCudaInteropMode::D3D11_INTEROP;
	for (PxU32 i = 0; i < PxCudaBufferMemorySpace::COUNT; i++)
	{
		desc.memoryBaseSize[i] = 0;
		desc.memoryPageSize[i] = 2 * 1024 * 1024;
		desc.maxMemorySize[i] = PX_MAX_U32;
	}
	m_pCudaContextManager = PxCreateCudaContextManager(*m_pFoundation, desc);

	if (m_pCudaContextManager && !m_pCudaContextManager->contextIsValid())
	{
		m_pCudaContextManager->release();
		m_pCudaContextManager = nullptr;
	}
	return true;
#else
	FLUX_LOG(Error, "[PhysicsSystem::InitializeCuda] Could not initialize Cuda because graphics platform is not supported");
	return false;
#endif
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
	pairFlags |= PxPairFlag::eCONTACT_DEFAULT;

	// trigger the contact callback for pairs (A,B) where
	// the filtermask of A contains the ID of B and vice versa.
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
	{
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
		pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
	}

	return PxFilterFlag::eDEFAULT;
}
