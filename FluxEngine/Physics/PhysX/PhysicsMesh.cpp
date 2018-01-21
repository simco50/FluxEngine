#include "FluxEngine.h"
#include "PhysicsMesh.h"
#include "FileSystem\File\File.h"
#include "PhysicsSystem.h"

PhysicsMesh::PhysicsMesh(PhysicsSystem* pPhysicsSystem):
	m_pPhysicsSystem(pPhysicsSystem)
{

}

PhysicsMesh::~PhysicsMesh()
{

}

bool PhysicsMesh::Load(const std::string& filePath)
{
	unique_ptr<IFile> pFile = FileSystem::GetFile(filePath);
	if (pFile == nullptr)
		return false;
	if (!pFile->Open(FileMode::Read, ContentType::Binary))
		return false;

	std::string geometryType = pFile->ReadSizedString();
	unsigned int byteSize;
	*pFile >> byteSize;

	InputStream inputStream(pFile.get());

	if (geometryType == "CONVEXMESH")
	{
		m_pMesh = m_pPhysicsSystem->GetPhysics()->createConvexMesh(inputStream);
		m_Type = Type::ConvexMesh;
	}
	else if (geometryType == "TRIANGLEMESH")
	{
		m_pMesh = m_pPhysicsSystem->GetPhysics()->createTriangleMesh(inputStream);
		m_Type = Type::TriangleMesh;
	}
	else
	{
		FLUX_LOG(ERROR, "[PhysicsMesh::Load] > Geometry type '%s' is unsupported", geometryType.c_str());
		return false;
	}
	return m_pMesh != nullptr;
}