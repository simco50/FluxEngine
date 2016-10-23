#pragma once

class MeshFilter;

#pragma warning(push)
#pragma warning(disable: 4505)

namespace FlexHelper
{
	struct FlexData
	{
		explicit FlexData()
		{
			//Offsets always start with 0
			RigidOffsets.push_back(0);
		}

		FlexParams Params;

		//Position + InvWeight
		vector<Vector4> Positions;
		//Holds all the velocities
		vector<Vector3> Velocities;
		//Holds all the phases
		vector<int> Phases;
		//The particle positions in their rest state
		vector<Vector4> RestPositions;

		//Rigids
		//Tells how many particles belong to which rigid cluster
		vector<int> RigidOffsets;
		//Holds indices of particles to appoint them to a cluster
		vector<int> RigidIndices;
		//Holds the transforms of the clusters
		vector<Vector3> RigidTranslations;
		vector<Vector4> RigidRotations;
		vector<float> RigidCoefficients;

		//Springs
		vector<int> SpringIndices;
		vector<float> SpringStiffness;
		vector<float> SpringLengths;
	};

	struct FlexMeshInstance
	{
		MeshFilter* pMeshFilter;
		vector<Vector3> RigidRestPoses;
		int Offset;
		int ParticleStart;
		int ParticleCount;

		//Skinning
		vector<int> SkinningIndices;
		vector<float> SkinningWeights;
	};

	static void FlexMessageCallback(FlexErrorSeverity type, const char* msg, const char* file, int line)
	{
		UNREFERENCED_PARAMETER(file);
		UNREFERENCED_PARAMETER(line);

		wstringstream str;
		str << "[Nvidia Flex] " << msg;
		switch (type)
		{
		case eFlexLogError:
			DebugLog::Log(str.str(), LogType::ERROR);
			break;
		case eFlexLogInfo:
			DebugLog::Log(str.str(), LogType::INFO);
			break;
		case eFlexLogWarning:
			DebugLog::Log(str.str(), LogType::WARNING);
			break;
		case eFlexLogDebug:
			DebugLog::Log(str.str(), LogType::INFO);
			break;
		case eFlexLogAll:
			break;
		default:
			break;
		}
	}
	
	/**
	 * param[out]  flexParams 
	 * Default radius is 0.15f
	 * Default earth gravity (-9.81f)
	 */
	static void SetDefaultParams(FlexParams &flexParams)
	{
		flexParams.mRadius = 0.15f;

		flexParams.mGravity[0] = 0.0f;
		flexParams.mGravity[1] = -9.8f;
		flexParams.mGravity[2] = 0.0f;

		flexParams.mWind[0] = 0.0f;
		flexParams.mWind[1] = 0.0f;
		flexParams.mWind[2] = 0.0f;

		flexParams.mViscosity = 0.0f;
		flexParams.mDynamicFriction = 0.0f;
		flexParams.mStaticFriction = 0.0f;
		flexParams.mParticleFriction = 0.0f; // scale friction between particles by default
		flexParams.mFreeSurfaceDrag = 0.0f;
		flexParams.mDrag = 0.0f;
		flexParams.mLift = 0.0f;
		flexParams.mNumIterations = 3;
		flexParams.mFluidRestDistance = 0.0f;
		flexParams.mSolidRestDistance = 0.0f;
		flexParams.mAnisotropyScale = 1.0f;
		flexParams.mAnisotropyMin = 0.1f;
		flexParams.mAnisotropyMax = 2.0f;
		flexParams.mSmoothing = 1.0f;
		flexParams.mDissipation = 0.0f;
		flexParams.mDamping = 0.0f;
		flexParams.mParticleCollisionMargin = 0.0f;
		flexParams.mShapeCollisionMargin = 0.0f;
		flexParams.mCollisionDistance = 0.0f;
		flexParams.mPlasticThreshold = 0.0f;
		flexParams.mPlasticCreep = 0.0f;
		flexParams.mFluid = false;
		flexParams.mSleepThreshold = 0.0f;
		flexParams.mShockPropagation = 0.0f;
		flexParams.mRestitution = 0.0f;
		flexParams.mMaxSpeed = FLT_MAX;
		flexParams.mRelaxationMode = eFlexRelaxationLocal;
		flexParams.mRelaxationFactor = 1.0f;
		flexParams.mSolidPressure = 1.0f;
		flexParams.mAdhesion = 0.0f;
		flexParams.mCohesion = 0.025f;
		flexParams.mSurfaceTension = 0.0f;
		flexParams.mVorticityConfinement = 0.0f;
		flexParams.mBuoyancy = 1.0f;

		flexParams.mDiffuseThreshold = 100.0f;
		flexParams.mDiffuseBuoyancy = 1.0f;
		flexParams.mDiffuseDrag = 0.8f;
		flexParams.mDiffuseBallistic = 16;
		flexParams.mDiffuseSortAxis[0] = 0.0f;
		flexParams.mDiffuseSortAxis[1] = 0.0f;
		flexParams.mDiffuseSortAxis[2] = 0.0f;
		flexParams.mDiffuseLifetime = 2.0f;

		flexParams.mInertiaBias = 0.001f;

		flexParams.mNumPlanes = 0;
	}

	/**
	 * Adds a plane to the flexParams
	 * param[out] params Reference to the flexParams
	 * param[in] normal The normal of the plane
	 * param[in] distance The distance of the plane (in the normal direction)
	 */
	static void CreateGroundPlane(FlexParams &params, const Vector3 &normal = Vector3(0,1,0), const float distance = 0.0f)
	{
		int planeIdx = params.mNumPlanes;
		++params.mNumPlanes;
		reinterpret_cast<Vector4&>(params.mPlanes[planeIdx]) = Vector4(normal.x, normal.y, normal.z, -distance);
	}

	static void AdjustParams(FlexParams& flexParams)
	{
		// by default solid particles use the maximum radius
		if (flexParams.mFluid && flexParams.mSolidRestDistance == 0.0f)
			flexParams.mSolidRestDistance = flexParams.mFluidRestDistance;
		else
			flexParams.mSolidRestDistance = flexParams.mRadius;
		// collision distance with shapes half the radius
		if (flexParams.mCollisionDistance == 0.0f)
		{
			flexParams.mCollisionDistance = flexParams.mRadius*0.5f;

			if (flexParams.mFluid)
				flexParams.mCollisionDistance = flexParams.mFluidRestDistance*0.5f;
		}
		// default particle friction to 10% of shape friction
		if (flexParams.mParticleFriction == 0.0f)
			flexParams.mParticleFriction = flexParams.mDynamicFriction*0.1f;
		// add a margin for detecting contacts between particles and shapes
		if (flexParams.mShapeCollisionMargin == 0.0f)
			flexParams.mShapeCollisionMargin = flexParams.mCollisionDistance*0.5f;
	}

		/** Transforms all the particles relative to the space of their cluster center (one particle can be part of multiple clusters)
	 * [in]param flexData The positions of the particles (4*n)s
	 * [in]param numRigids The amount of rigids
	 * [out]param localPositions
	 */
	static void CalculateRigidOffsets(const FlexData& flexData, const int numRigids, Vector3* localPositions)
	{
		int count = 0;

		for (int r = 0; r < numRigids; ++r)
		{
			const int startIndex = flexData.RigidOffsets[r];
			const int endIndex = flexData.RigidOffsets[r + 1];

			const int clusterSize = endIndex - startIndex;

			Vector3 average = Vector3();
			for (int i = startIndex; i < endIndex; ++i)
			{
				const int rigidIndex = flexData.RigidIndices[i];
				average = average + Vector3((float*)&flexData.Positions[rigidIndex]);
			}

			average = average / float(clusterSize);

			for (int i = startIndex; i < endIndex; ++i)
			{
				const int rigidIdx = flexData.RigidIndices[i];
				//Subtract the average position from the particle position to get the local position
				localPositions[count++] = Vector3((float*)&flexData.Positions[rigidIdx]) - average;
			}
		}
	}
}
#pragma warning(pop)