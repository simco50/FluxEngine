#pragma once
#include "PhysxAllocator.h"
#include "PhysxErrorCallback.h"

class PhysicsSystem
{
public:
	PhysicsSystem();
	~PhysicsSystem();

	void Initialize();
	void SetupPvdConnection();
	void Shutdown();

	physx::PxPhysics* GetPhysics() const { return m_pPhysics; }
	physx::PxFoundation* GetFoundation() const { return m_pFoundation; }

private:
	PhysxAllocator m_AllocatorCallback;
	PhysxErrorCallback m_ErrorCallback;

	physx::PxFoundation* m_pFoundation = nullptr;
	physx::PxPhysics* m_pPhysics = nullptr;
	physx::PxPvd* m_pPvd = nullptr;
	physx::PxPvdTransport* m_pPvdTransport = nullptr;
};