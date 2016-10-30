#include "stdafx.h"
#include "FlexSystem.h"

FlexSystem::FlexSystem()
{
	RigidOffsets.push_back(0);
}


FlexSystem::~FlexSystem()
{
	if(pFlexSolver)
		flexDestroySolver(pFlexSolver);
}

void FlexSystem::InitializeSolver()
{
	pFlexSolver = flexCreateSolver(Positions.size(), 0);
}

void FlexSystem::FetchData()
{
	flexGetRigidTransforms(pFlexSolver, (float*)RigidRotations.data(), (float*)RigidTranslations.data(), MemoryType);
	flexGetParticles(pFlexSolver, (float*)Positions.data(), Positions.size(), MemoryType);
	flexGetVelocities(pFlexSolver, (float*)Velocities.data(), Velocities.size(), MemoryType);
}

void FlexSystem::UpdateSolver(int substeps, float deltaTime)
{
	flexUpdateSolver(pFlexSolver, deltaTime, substeps, nullptr);
}

void FlexSystem::UpdateData()
{
	flexSetParams(pFlexSolver, &Params);

	const int numParticles = Positions.size();

	flexSetParticles(pFlexSolver, (float*)Positions.data(), numParticles, MemoryType);
	flexSetVelocities(pFlexSolver, (float*)Velocities.data(), numParticles, MemoryType);

	//Convex shapes
	if (ShapePositions.size() > 0)
	{
		flexSetShapes(pFlexSolver,
			ShapeGeometry.data(),
			ShapeGeometry.size(),
			(float*)ShapeAabbMin.data(),
			(float*)ShapeAabbMax.data(),
			ShapeStarts.data(),
			(float*)ShapePositions.data(),
			(float*)ShapeRotations.data(),
			(float*)ShapePrevPositions.data(),
			(float*)ShapePrevRotations.data(),
			ShapeFlags.data(),
			ShapeStarts.size(),
			MemoryType
		);
	}

	flexSetPhases(pFlexSolver, Phases.data(), numParticles, MemoryType);
}

void FlexSystem::UploadFlexData()
{
	InitializeSolver();

	//Adjust the flex params
	Params.mRadius *= 1.5f;
	AdjustParams();
	flexSetParams(pFlexSolver, &Params);

	const int numParticles = Positions.size();

	flexSetParticles(pFlexSolver, (float*)Positions.data(), numParticles, MemoryType);
	flexSetVelocities(pFlexSolver, (float*)Velocities.data(), numParticles, MemoryType);

	vector<int> activeIndices(numParticles);
	for (int i = 0; i < numParticles; i++)
		activeIndices[i] = i;
	flexSetActive(pFlexSolver, activeIndices.data(), numParticles, MemoryType);

	//Rigids
	if (RigidOffsets.size() > 1)
	{
		const int numRigids = RigidOffsets.size() - 1;
		vector<Vector3> rigidLocalPositions(RigidOffsets.back());

		// calculate local rest space positions
		CalculateRigidOffsets(numRigids, &rigidLocalPositions[0]);

		RigidRotations.resize(RigidOffsets.size() - 1, Vector4());
		RigidTranslations.resize(RigidOffsets.size() - 1, Vector3());

		flexSetRigids(
			pFlexSolver,
			&RigidOffsets[0],
			&RigidIndices[0],
			(float*)rigidLocalPositions.data(),
			nullptr,
			RigidCoefficients.data(),
			(float*)&RigidRotations[0],
			(float*)&RigidTranslations[0],
			numRigids,
			MemoryType
		);
	}

	//Springs
	if (SpringIndices.size() > 0)
		flexSetSprings(pFlexSolver, SpringIndices.data(), SpringLengths.data(), SpringStiffness.data(), SpringLengths.size(), MemoryType);

	//Convex shapes
	if (ShapePositions.size() > 0)
	{
		flexSetShapes(pFlexSolver,
		              ShapeGeometry.data(),
		              ShapeGeometry.size(),
		              (float*)ShapeAabbMin.data(),
		              (float*)ShapeAabbMax.data(),
		              ShapeStarts.data(),
		              (float*)ShapePositions.data(),
		              (float*)ShapeRotations.data(),
		              (float*)ShapePrevPositions.data(),
		              (float*)ShapePrevRotations.data(),
		              ShapeFlags.data(),
		              ShapeStarts.size(),
		              MemoryType
		);
	}

	flexSetPhases(pFlexSolver, Phases.data(), numParticles, MemoryType);

	RestPositions.insert(RestPositions.begin(), Positions.begin(), Positions.end());
	flexSetRestParticles(pFlexSolver, (float*)RestPositions.data(), numParticles, MemoryType);

	DebugLog::LogFormat(LogType::INFO, L"FlexSystem::UploadFlexData() > Flex data uploaded successfully: %i particles", Positions.size());
}

void FlexSystem::SetDefaultParams()
{
	Params.mRadius = 0.15f;

	Params.mGravity[0] = 0.0f;
	Params.mGravity[1] = -9.8f;
	Params.mGravity[2] = 0.0f;

	Params.mWind[0] = 0.0f;
	Params.mWind[1] = 0.0f;
	Params.mWind[2] = 0.0f;

	Params.mViscosity = 0.0f;
	Params.mDynamicFriction = 0.0f;
	Params.mStaticFriction = 0.0f;
	Params.mParticleFriction = 0.0f; // scale friction between particles by default
	Params.mFreeSurfaceDrag = 0.0f;
	Params.mDrag = 0.0f;
	Params.mLift = 0.0f;
	Params.mNumIterations = 3;
	Params.mFluidRestDistance = 0.0f;
	Params.mSolidRestDistance = 0.0f;
	Params.mAnisotropyScale = 1.0f;
	Params.mAnisotropyMin = 0.1f;
	Params.mAnisotropyMax = 2.0f;
	Params.mSmoothing = 1.0f;
	Params.mDissipation = 0.0f;
	Params.mDamping = 0.0f;
	Params.mParticleCollisionMargin = 0.0f;
	Params.mShapeCollisionMargin = 0.0f;
	Params.mCollisionDistance = 0.0f;
	Params.mPlasticThreshold = 0.0f;
	Params.mPlasticCreep = 0.0f;
	Params.mFluid = false;
	Params.mSleepThreshold = 0.0f;
	Params.mShockPropagation = 0.0f;
	Params.mRestitution = 0.0f;
	Params.mMaxSpeed = FLT_MAX;
	Params.mRelaxationMode = eFlexRelaxationLocal;
	Params.mRelaxationFactor = 1.0f;
	Params.mSolidPressure = 1.0f;
	Params.mAdhesion = 0.0f;
	Params.mCohesion = 0.025f;
	Params.mSurfaceTension = 0.0f;
	Params.mVorticityConfinement = 0.0f;
	Params.mBuoyancy = 1.0f;

	Params.mDiffuseThreshold = 100.0f;
	Params.mDiffuseBuoyancy = 1.0f;
	Params.mDiffuseDrag = 0.8f;
	Params.mDiffuseBallistic = 16;
	Params.mDiffuseSortAxis[0] = 0.0f;
	Params.mDiffuseSortAxis[1] = 0.0f;
	Params.mDiffuseSortAxis[2] = 0.0f;
	Params.mDiffuseLifetime = 2.0f;

	Params.mInertiaBias = 0.001f;

	Params.mNumPlanes = 0;
}

void FlexSystem::CreateGroundPlane(const Vector3& normal, const float distance)
{
	int planeIdx = Params.mNumPlanes;
	++Params.mNumPlanes;
	reinterpret_cast<Vector4&>(Params.mPlanes[planeIdx]) = Vector4(normal.x, normal.y, normal.z, -distance);
}

void FlexSystem::AdjustParams()
{
	// by default solid particles use the maximum radius
	if (Params.mFluid && Params.mSolidRestDistance == 0.0f)
		Params.mSolidRestDistance = Params.mFluidRestDistance;
	else
		Params.mSolidRestDistance = Params.mRadius;
	// collision distance with shapes half the radius
	if (Params.mCollisionDistance == 0.0f)
	{
		Params.mCollisionDistance = Params.mRadius * 0.5f;

		if (Params.mFluid)
			Params.mCollisionDistance = Params.mFluidRestDistance * 0.5f;
	}
	// default particle friction to 10% of shape friction
	if (Params.mParticleFriction == 0.0f)
		Params.mParticleFriction = Params.mDynamicFriction * 0.1f;
	// add a margin for detecting contacts between particles and shapes
	if (Params.mShapeCollisionMargin == 0.0f)
		Params.mShapeCollisionMargin = Params.mCollisionDistance * 0.5f;
}

void FlexSystem::CalculateRigidOffsets(const int numRigids, Vector3* localPositions)
{
	int count = 0;

	for (int r = 0; r < numRigids; ++r)
	{
		const int startIndex = RigidOffsets[r];
		const int endIndex = RigidOffsets[r + 1];

		const int clusterSize = endIndex - startIndex;

		Vector3 average = Vector3();
		for (int i = startIndex; i < endIndex; ++i)
		{
			const int rigidIndex = RigidIndices[i];
			average = average + Vector3((float*)&Positions[rigidIndex]);
		}

		average = average / float(clusterSize);

		for (int i = startIndex; i < endIndex; ++i)
		{
			const int rigidIdx = RigidIndices[i];
			//Subtract the average position from the particle position to get the local position
			localPositions[count++] = Vector3((float*)&Positions[rigidIdx]) - average;
		}
	}
}