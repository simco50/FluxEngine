#include "FluxEngine.h"
#include "Cloth.h"
#include "PhysicsSystem.h"
#include "Scenegraph\SceneNode.h"
#include "Scenegraph\Transform.h"
#include "Scenegraph\Scene.h"
#include "PhysicsScene.h"
#include "Rendering\Geometry.h"

Cloth::Cloth(Context* pContext) :
	Component(pContext)
{
	m_pPhysicsSystem = pContext->GetSubsystem<PhysicsSystem>();
}

Cloth::~Cloth()
{
}

void Cloth::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);
	CreateCloth();
}

void Cloth::OnNodeRemoved()
{
	Component::OnNodeRemoved();
}

void Cloth::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);
	m_pPhysicsScene = pScene->GetOrCreateComponent<PhysicsScene>();
	CreateCloth();
}

void Cloth::SetGeometry(Geometry* pGeometry)
{
	if (pGeometry == m_pGeometry)
		return;
	m_pGeometry = pGeometry;
	if (m_pCloth && m_pPhysicsScene)
		m_pPhysicsScene->GetScene()->removeActor(*m_pCloth);
	m_pCloth = nullptr;
	CreateCloth();
}

void Cloth::CreateCloth()
{
	if (m_pGeometry == nullptr || m_pPhysicsScene == nullptr)
		return;

	PxClothMeshDesc meshDesc = {};

	std::vector<PxClothParticle> particles(m_pGeometry->GetVertexCount());

	const Geometry::VertexData* pColorData = nullptr;
	if(m_pGeometry->HasData("COLOR"))
		pColorData = &m_pGeometry->GetVertexData("COLOR");

	const Geometry::VertexData& positionData = m_pGeometry->GetVertexData("POSITION");
	PxClothParticle* pParticle = reinterpret_cast<PxClothParticle*>(particles.data());
	for (int i = 0; i < positionData.Count; ++i)
	{
		pParticle[i].pos = static_cast<PxVec3*>(positionData.pData)[i];
		if (pColorData)
			pParticle[i].invWeight = 1.0f - static_cast<Color*>(pColorData->pData)[i].R();
		else
			pParticle[i].invWeight = 1.0f;
	}

	if (m_pGeometry->HasData("INDEX"))
	{
		const Geometry::VertexData& indexData = m_pGeometry->GetVertexData("INDEX");
		meshDesc.triangles.data = indexData.pData;
		meshDesc.triangles.count = indexData.Count / 3;
		meshDesc.triangles.stride = indexData.Stride * 3;
	}
	else
	{
		std::vector<int> indices(positionData.Count);
		for (unsigned int i = 0; i < indices.size(); ++i)
			indices[i] = i;

		meshDesc.triangles.data = indices.data();
		meshDesc.triangles.count = (PxU32)indices.size() / 3;
		meshDesc.triangles.stride = sizeof(unsigned int) * 3;
	}

	meshDesc.points.data = particles.data();
	meshDesc.points.count = (PxU32)particles.size();
	meshDesc.points.stride = sizeof(PxClothParticle);

	meshDesc.invMasses.data = &particles[0].invWeight;
	meshDesc.invMasses.count = (PxU32)particles.size();
	meshDesc.invMasses.stride = sizeof(PxClothParticle);

	PxClothFabric* pFabric = PxClothFabricCreate(*m_pPhysicsSystem->GetPhysics(), meshDesc, PxVec3(0, -1, 0));
	PxTransform pose(*reinterpret_cast<const PxMat44*>(&m_pNode->GetTransform()->GetWorldMatrix()));
	m_pCloth = m_pPhysicsSystem->GetPhysics()->createCloth(pose, *pFabric, particles.data(), PxClothFlags());
	
	PxClothStretchConfig stretchConfig;
	stretchConfig.stiffness = 0.2f;
	stretchConfig.stiffnessMultiplier = 0.0f;
	stretchConfig.compressionLimit = 0.0f;
	stretchConfig.stretchLimit = 1.0f;
	m_pCloth->setStretchConfig(PxClothFabricPhaseType::eHORIZONTAL, stretchConfig);
	m_pCloth->setStretchConfig(PxClothFabricPhaseType::eVERTICAL, stretchConfig);
	m_pCloth->setDragCoefficient(1.0f);
	m_pCloth->setSelfCollisionDistance(0.0f);
	m_pCloth->setSolverFrequency(120);

	m_pPhysicsScene->GetScene()->addActor(*m_pCloth);

	m_pCloth->setExternalAcceleration(PxVec3(0, 0, 1));

	PhysXSafeRelease(pFabric);
}