#pragma once
#include "Content/Resource.h"

class PhysicsSystem;

class PhysicsMesh : public Resource
{
	FLUX_OBJECT(PhysicsMesh, Resource)

public:
	PhysicsMesh(Context* pContext);
	virtual ~PhysicsMesh();

	virtual bool Load(InputStream& inputStream) override;

	physx::PxBase* GetMesh() const { return m_pMesh; }

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

	class PhysxInputStream : public physx::PxInputStream
	{
	public:
		PhysxInputStream(InputStream& inputStream) :
			m_pStream(&inputStream)
		{}

		virtual uint32_t read(void* dest, uint32_t count) override
		{
			return (uint32_t)m_pStream->Read((char*)dest, (size_t)count);
		}

	private:
		InputStream* m_pStream;
	};

	physx::PxBase* m_pMesh = nullptr;
};