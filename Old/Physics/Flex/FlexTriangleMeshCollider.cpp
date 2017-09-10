#include "stdafx.h"
#include "FlexTriangleMeshCollider.h"
#include "FlexSystem.h"
#include "../../Scenegraph/GameObject.h"
#include "../../Components/MeshComponent.h"
#include "../../Graphics/MeshFilter.h"
#include "../../Components/TransformComponent.h"

FlexTriangleMeshCollider::FlexTriangleMeshCollider(FlexSystem* pFlexSystem) : m_pFlexSystem(pFlexSystem)
{
}


FlexTriangleMeshCollider::~FlexTriangleMeshCollider()
{
}

void FlexTriangleMeshCollider::Initialize()
{
	MeshComponent* pMeshComponent = m_pGameObject->GetComponent<MeshComponent>();
	if (pMeshComponent == nullptr)
		Console::Log("FlexTriangleMeshCollider::Initialize() > Object has no MeshComponent!", LogType::ERROR);
	MeshFilter* pMeshFilter = pMeshComponent->GetMeshFilter();
	if (pMeshFilter == nullptr)
		Console::Log("FlexTriangleMeshCollider::Initialize() > Object has no MeshFilter!", LogType::ERROR);
	m_pTransformComponent = m_pGameObject->GetTransform();
	if (m_pTransformComponent == nullptr)
		Console::Log("FlexTriangleMeshCollider::Initialize() > Object has no TransformComponent!", LogType::ERROR);

	FlexTriangleMesh* pMesh = flexCreateTriangleMesh();

	BoundingBox::CreateFromPoints(m_BoundingBox, pMeshFilter->VertexCount(), (XMFLOAT3*)pMeshFilter->GetVertexData("POSITION").pData, sizeof(XMFLOAT3));

	Vector3 min = m_BoundingBox.Center - m_BoundingBox.Extents;
	Vector3 max = m_BoundingBox.Center + m_BoundingBox.Extents;

	flexUpdateTriangleMesh(pMesh,
		(float*)pMeshFilter->GetVertexData("POSITION").pData,
		(int*)pMeshFilter->GetVertexData("INDEX").pData,
		pMeshFilter->VertexCount(),
		pMeshFilter->IndexCount() / 3,
		(float*)&min,
		(float*)&max,
		m_pFlexSystem->MemoryType
	);

	FlexCollisionGeometry geometry;
	geometry.mTriMesh.mMesh = pMesh;
	geometry.mTriMesh.mScale = 1.0f;

	m_ShapeIdx = m_pFlexSystem->ShapeGeometry.size();
	m_pFlexSystem->ShapeStarts.push_back(m_ShapeIdx);
	m_pFlexSystem->ShapeAabbMin.push_back(Vector4(min.x, min.y, min.z, 0.0f));
	m_pFlexSystem->ShapeAabbMax.push_back(Vector4(max.x, max.y, max.z, 0.0f));
	m_pFlexSystem->ShapePositions.push_back(Vector4(0, 0, 0, 0.0f));
	m_pFlexSystem->ShapeRotations.push_back(Quaternion());
	m_pFlexSystem->ShapePrevPositions.push_back(Vector4(0, 0, 0, 0.0f));
	m_pFlexSystem->ShapePrevRotations.push_back(Quaternion());
	m_pFlexSystem->ShapeGeometry.push_back(geometry);
	m_pFlexSystem->ShapeFlags.push_back(flexMakeShapeFlags(eFlexShapeTriangleMesh, false));
}

void FlexTriangleMeshCollider::Update()
{
	BoundingBox bounds;
	m_BoundingBox.Transform(bounds, Matrix(m_pTransformComponent->GetWorldMatrix()));

	m_pFlexSystem->ShapePrevPositions[m_ShapeIdx] = m_pFlexSystem->ShapePositions[m_ShapeIdx];
	Vector3 worldPos = m_pTransformComponent->GetWorldPosition();
	m_pFlexSystem->ShapePositions[m_ShapeIdx] = Vector4(worldPos.x, worldPos.y, worldPos.z, 0);
	m_pFlexSystem->ShapePrevRotations[m_ShapeIdx] = m_pFlexSystem->ShapeRotations[m_ShapeIdx];
	m_pFlexSystem->ShapeRotations[m_ShapeIdx] = m_pTransformComponent->GetWorldRotation();

	Vector3 min = bounds.Center - bounds.Extents;
	Vector3 max = bounds.Center + bounds.Extents;
	m_pFlexSystem->ShapeAabbMin[m_ShapeIdx] = Vector4(min.x, min.y, min.z, 0);
	m_pFlexSystem->ShapeAabbMax[m_ShapeIdx] = Vector4(max.x, max.y, max.z, 0);

	m_pFlexSystem->ShapeGeometry[m_ShapeIdx].mTriMesh.mScale = m_pTransformComponent->GetScale().x;
}

void FlexTriangleMeshCollider::Render()
{
}