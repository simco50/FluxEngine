#include "FluxEngine.h"
#include "PhysicsMesh.h"
#include "PhysicsSystem.h"
#include "IO/InputStream.h"

PhysicsMesh::PhysicsMesh(Context* pContext):
	Resource(pContext)
{
	m_pPhysicsSystem = pContext->GetSubsystem<PhysicsSystem>();
}

PhysicsMesh::~PhysicsMesh()
{

}

bool PhysicsMesh::Load(InputStream& inputStream)
{
	std::string geometryType = inputStream.ReadSizedString();
	unsigned int byteSize;
	inputStream.Read(&byteSize, sizeof(unsigned int));

	PhysxInputStream physicsInputStream(inputStream);

	if (geometryType == "CONVEXMESH")
	{
		m_pMesh = m_pPhysicsSystem->GetPhysics()->createConvexMesh(physicsInputStream);
		m_Type = Type::ConvexMesh;
	}
	else if (geometryType == "TRIANGLEMESH")
	{
		m_pMesh = m_pPhysicsSystem->GetPhysics()->createTriangleMesh(physicsInputStream);
		m_Type = Type::TriangleMesh;
	}
	else
	{
		FLUX_LOG(Error, "[PhysicsMesh::Load] > Geometry type '%s' is unsupported", geometryType.c_str());
		return false;
	}
	return m_pMesh != nullptr;
}
