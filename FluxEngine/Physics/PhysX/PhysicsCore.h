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
	PxProfileZoneManager* m_pProfileZoneManager = nullptr;
	PxFoundation* m_pFoundation = nullptr;
	PxPhysics* m_pPhysics = nullptr;
	PxScene* m_pScene = nullptr;
	PxDefaultCpuDispatcher* m_pDefaultCpuDispatcher = nullptr;

	PxVisualDebuggerConnection* m_pPvdConnection = nullptr;

	FluxPhysxErrorCallback m_ErrorCallback;
	FluxPhysxAllocatorCallback m_AllocatorCallback;
};

