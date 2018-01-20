#pragma once
#include "PhysxAllocator.h"
#include "PhysxErrorCallback.h"

class PhysicsSystem
{
public:
	PhysicsSystem();
	~PhysicsSystem();

	physx::PxPhysics* GetPhysics() const { return m_pPhysics; }
	physx::PxFoundation* GetFoundation() const { return m_pFoundation; }
	physx::PxCpuDispatcher* GetCpuDispatcher() const { return m_pCpuDispatcher; }
	physx::PxPvd* GetPvd() const { return m_pPvd; }

	static PxFilterFlags SimulationFilterShader(
		PxFilterObjectAttributes attribute0, PxFilterData filterData0,
		PxFilterObjectAttributes attribute1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);

	physx::PxMaterial* GetDefaultMaterial() const { return m_pDefaultMaterial; }

private:
	PhysxAllocator m_AllocatorCallback;
	PhysxErrorCallback m_ErrorCallback;

	physx::PxCpuDispatcher* m_pCpuDispatcher = nullptr;

	physx::PxFoundation* m_pFoundation = nullptr;
	physx::PxPhysics* m_pPhysics = nullptr;
	physx::PxPvd* m_pPvd = nullptr;
	physx::PxPvdTransport* m_pPvdTransport = nullptr;

	physx::PxMaterial* m_pDefaultMaterial = nullptr;
};