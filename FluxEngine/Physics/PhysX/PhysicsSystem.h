#pragma once
#include "PhysxAllocator.h"
#include "PhysxErrorCallback.h"
#include "Core\Subsystem.h"

class Graphics;

class PhysicsSystem : public Subsystem
{
	FLUX_OBJECT(PhysicsSystem, Subsystem)

public:
	PhysicsSystem(Context* pContext, Graphics* pGraphics = nullptr);
	~PhysicsSystem();

	physx::PxPhysics* GetPhysics() const { return m_pPhysics; }
	physx::PxFoundation* GetFoundation() const { return m_pFoundation; }
	physx::PxCpuDispatcher* GetCpuDispatcher() const { return m_pCpuDispatcher; }
	physx::PxPvd* GetPvd() const { return m_pPvd; }
	physx::PxCudaContextManager* GetCudaContextManager() const { return m_pCudaContextManager; }

	static physx::PxFilterFlags SimulationFilterShader(
		physx::PxFilterObjectAttributes attribute0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attribute1, physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

	physx::PxMaterial* GetDefaultMaterial() const { return m_pDefaultMaterial; }

private:
	PhysxAllocator m_AllocatorCallback;
	PhysxErrorCallback m_ErrorCallback;

	physx::PxCpuDispatcher* m_pCpuDispatcher = nullptr;

	physx::PxFoundation* m_pFoundation = nullptr;
	physx::PxPhysics* m_pPhysics = nullptr;
	physx::PxPvd* m_pPvd = nullptr;
	physx::PxPvdTransport* m_pPvdTransport = nullptr;
	physx::PxCudaContextManager* m_pCudaContextManager = nullptr;

	physx::PxMaterial* m_pDefaultMaterial = nullptr;
};