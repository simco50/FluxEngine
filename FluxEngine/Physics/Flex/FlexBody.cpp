#include "stdafx.h"
#include "FlexBody.h"
#include "FlexHelper.h"

FlexBody::FlexBody(const wstring& filePath, FlexHelper::FlexData* pFlexData):
m_pFlexData(pFlexData), m_FilePath(filePath)
{
}

FlexBody::~FlexBody()
{
}

void FlexBody::SetPosition(const Vector3& position)
{
	Vector3 avg = Vector3();

	for (int i = m_pMeshInstance->ParticleStart; i < m_pMeshInstance->ParticleStart + m_pMeshInstance->ParticleCount; ++i)
	{
		avg.x += m_pFlexData->Positions[i].x;
		avg.y += m_pFlexData->Positions[i].y;
		avg.z += m_pFlexData->Positions[i].z;
	}
	avg /= (float)m_pMeshInstance->ParticleCount;

	Vector3 delta = position - avg;
	for (int i = m_pMeshInstance->ParticleStart; i < m_pMeshInstance->ParticleStart + m_pMeshInstance->ParticleCount; ++i)
	{
		m_pFlexData->Positions[i].x += delta.x;
		m_pFlexData->Positions[i].y += delta.y;
		m_pFlexData->Positions[i].z += delta.z;
		m_pFlexData->Velocities[i] = Vector3(0, 0, 0);
	}
}

void FlexBody::AddBodyFlexData(FlexExtAsset* pAsset, const int phase)
{
	const int particleOffset = m_pFlexData->Positions.size();
	const int indexOffset = m_pFlexData->RigidOffsets.back();

	m_pMeshInstance->Offset = m_pFlexData->RigidTranslations.size();
	m_pMeshInstance->ParticleStart = m_pFlexData->Positions.size();
	m_pMeshInstance->ParticleCount = pAsset->mNumParticles;

	//Set the individual particle data
	for (int i = 0; i < pAsset->mNumParticles; i++)
	{
		m_pFlexData->Positions.push_back(Vector4(&pAsset->mParticles[i * 4]));
		m_pFlexData->Velocities.push_back(Vector3());
		m_pFlexData->Phases.push_back(phase);
	}

	for (int i = 0; i < pAsset->mNumShapeIndices; i++)
		m_pFlexData->RigidIndices.push_back(pAsset->mShapeIndices[i] + particleOffset);

	//Add the rigid configurations
	for (int i = 0; i < pAsset->mNumShapes; i++)
	{
		m_pFlexData->RigidOffsets.push_back(pAsset->mShapeOffsets[i] + indexOffset);
		m_pFlexData->RigidTranslations.push_back(Vector3(&pAsset->mShapeCenters[i * 3]));
		m_pFlexData->RigidRotations.push_back(Vector4());
		m_pFlexData->RigidCoefficients.push_back(pAsset->mShapeCoefficients[i]);
	}

	//Add the spring configurations
	for (int i = 0; i < pAsset->mNumSprings; i++)
	{
		m_pFlexData->SpringIndices.push_back(pAsset->mSpringIndices[i * 2]);
		m_pFlexData->SpringIndices.push_back(pAsset->mSpringIndices[i * 2 + 1]);
		m_pFlexData->SpringStiffness.push_back(pAsset->mSpringCoefficients[i]);
		m_pFlexData->SpringLengths.push_back(pAsset->mSpringRestLengths[i]);
	}
}
