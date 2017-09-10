#pragma once
#include "PhysxCallbacks.h"

class PhysicsCore
{
public:
	PhysicsCore();
	~PhysicsCore();

	void Initialize();
	void Update();
	void Finalize();

private:
	PxFoundation* m_pFoundation = nullptr;
	PxPhysics* m_pPhysics = nullptr;
	PxScene* m_pScene = nullptr;
	PxDefaultCpuDispatcher* m_pDefaultCpuDispatcher = nullptr;
	PxPvd* m_pPvdConnection = nullptr;
	PxPvdTransport* m_pPvdTransport = nullptr;
	FluxPhysxErrorCallback m_ErrorCallback;
	FluxPhysxAllocatorCallback m_AllocatorCallback;

	static PxMaterial* DefaultMaterial;
};

