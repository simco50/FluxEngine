#pragma once
#include "Content\Resource.h"

class PhysicsSystem;

class PhysicsMesh : public Resource
{
	FLUX_OBJECT(PhysicsMesh, Resource)

public:
	PhysicsMesh(PhysicsSystem* pPhysicsSystem);
	virtual ~PhysicsMesh();

	virtual bool Load(const std::string& filePath) override;

	PxBase* GetMesh() const { return m_pMesh; }

	enum class Type
	{
		ConvexMesh,
		TriangleMesh,
		None,
	};
	PhysicsMesh::Type GetMeshType() const { return m_Type; }

private:
	PhysicsSystem * m_pPhysicsSystem;
	Type m_Type = Type::None;

	class InputStream : public PxInputStream
	{
	public:
		InputStream(IFile* pFile) : 
			m_pFile(pFile)
		{}

		virtual uint32_t read(void* dest, uint32_t count) override
		{
			return m_pFile->Read((unsigned int)count, (char*)dest);
		}

	private:
		IFile * m_pFile;
	};

	PxBase* m_pMesh = nullptr;
};