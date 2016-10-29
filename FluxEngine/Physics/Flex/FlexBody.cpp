#include "stdafx.h"
#include "FlexBody.h"
#include "FlexHelper.h"
#include "../../FlexSystem.h"

FlexBody::FlexBody(const wstring& filePath, FlexSystem* pFlexSystem):
	m_pFlexSystem(pFlexSystem), m_FilePath(filePath)
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
		avg.x += m_pFlexSystem->Positions[i].x;
		avg.y += m_pFlexSystem->Positions[i].y;
		avg.z += m_pFlexSystem->Positions[i].z;
	}
	avg /= (float)m_pMeshInstance->ParticleCount;

	Vector3 delta = position - avg;
	for (int i = m_pMeshInstance->ParticleStart; i < m_pMeshInstance->ParticleStart + m_pMeshInstance->ParticleCount; ++i)
	{
		m_pFlexSystem->Positions[i].x += delta.x;
		m_pFlexSystem->Positions[i].y += delta.y;
		m_pFlexSystem->Positions[i].z += delta.z;
		m_pFlexSystem->Velocities[i] = Vector3(0, 0, 0);
	}
}

void FlexBody::AddBodyFlexData(FlexExtAsset* pAsset, const int phase)
{
	const int particleOffset = m_pFlexSystem->Positions.size();
	const int indexOffset = m_pFlexSystem->RigidOffsets.back();

	m_pMeshInstance->Offset = m_pFlexSystem->RigidTranslations.size();
	m_pMeshInstance->ParticleStart = m_pFlexSystem->Positions.size();
	m_pMeshInstance->ParticleCount = pAsset->mNumParticles;

	//Set the individual particle data
	for (int i = 0; i < pAsset->mNumParticles; i++)
	{
		m_pFlexSystem->Positions.push_back(Vector4(&pAsset->mParticles[i * 4]));
		m_pFlexSystem->Velocities.push_back(Vector3());
		m_pFlexSystem->Phases.push_back(phase);
	}

	for (int i = 0; i < pAsset->mNumShapeIndices; i++)
		m_pFlexSystem->RigidIndices.push_back(pAsset->mShapeIndices[i] + particleOffset);

	//Add the rigid configurations
	for (int i = 0; i < pAsset->mNumShapes; i++)
	{
		m_pFlexSystem->RigidOffsets.push_back(pAsset->mShapeOffsets[i] + indexOffset);
		m_pFlexSystem->RigidTranslations.push_back(Vector3(&pAsset->mShapeCenters[i * 3]));
		m_pFlexSystem->RigidRotations.push_back(Vector4());
		m_pFlexSystem->RigidCoefficients.push_back(pAsset->mShapeCoefficients[i]);
	}

	//Add the spring configurations
	for (int i = 0; i < pAsset->mNumSprings; i++)
	{
		m_pFlexSystem->SpringIndices.push_back(pAsset->mSpringIndices[i * 2]);
		m_pFlexSystem->SpringIndices.push_back(pAsset->mSpringIndices[i * 2 + 1]);
		m_pFlexSystem->SpringStiffness.push_back(pAsset->mSpringCoefficients[i]);
		m_pFlexSystem->SpringLengths.push_back(pAsset->mSpringRestLengths[i]);
	}
}
