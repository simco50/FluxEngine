#include "stdafx.h"
#include "FlexSystem.h"

FlexSystem::FlexSystem(NvFlexLibrary* pFlexLibrary) : pFlexLibrary(pFlexLibrary)
{
	RigidOffsets.push_back(0);
}

FlexSystem::~FlexSystem()
{
	if(pFlexSolver)
		NvFlexDestroySolver(pFlexSolver);
}

void FlexSystem::InitializeSolver()
{
	if (pFlexSolver)
	{
		Console::Log("FlexSolver already initialized, reinitializing...", LogType::WARNING);
		NvFlexDestroySolver(pFlexSolver);
		pFlexSolver = nullptr;
	}
	pFlexSolver = NvFlexCreateSolver(pFlexLibrary, Positions.size(), 0);
}

void FlexSystem::FetchData()
{
	NvFlexGetRigidTransforms(pFlexSolver, (float*)RigidRotations.data(), (float*)RigidTranslations.data());
	NvFlexGetParticles(pFlexSolver, (float*)Positions.data(), Positions.size());
	NvFlexGetVelocities(pFlexSolver, (float*)Velocities.data(), Velocities.size());
}

void FlexSystem::UpdateSolver(float deltaTime)
{
	NvFlexUpdateSolver(pFlexSolver, deltaTime, Substeps, false);
}

void FlexSystem::UpdateData()
{
	NvFlexSetParams(pFlexSolver, &Params);

	const int numParticles = Positions.size();

	NvFlexSetParticles(pFlexSolver, (float*)Positions.data(), numParticles);
	NvFlexSetVelocities(pFlexSolver, (float*)Velocities.data(), numParticles);

	//Convex shapes
	if (ShapePositions.size() > 0)
	{
		NvFlexSetShapes(pFlexSolver,
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
			ShapeStarts.size()
		);
	}
	NvFlexSetPhases(pFlexSolver, Phases.data(), numParticles);
}

void FlexSystem::UploadFlexData()
{
	if(pFlexSolver == nullptr)
	{
		Console::Log("FlexSystem::UploadFlexData() > Solver not yet initialized!", LogType::ERROR);
		return;
	}

	//Adjust the flex params
	Params.radius *= 1.5f;
	AdjustParams();
	NvFlexSetParams(pFlexSolver, &Params);

	const int numParticles = Positions.size();

	NvFlexSetParticles(pFlexSolver, (float*)Positions.data(), numParticles);
	NvFlexSetVelocities(pFlexSolver, (float*)Velocities.data(), numParticles);

	vector<int> activeIndices(numParticles);
	for (int i = 0; i < numParticles; i++)
		activeIndices[i] = i;
	NvFlexSetActive(pFlexSolver, activeIndices.data(), numParticles);

	//Rigids
	if (RigidOffsets.size() > 1)
	{
		const int numRigids = RigidOffsets.size() - 1;
		vector<Vector3> rigidLocalPositions(RigidOffsets.back());

		// calculate local rest space positions
		CalculateRigidOffsets(numRigids, &rigidLocalPositions[0]);

		RigidRotations.resize(RigidOffsets.size() - 1, Vector4());
		RigidTranslations.resize(RigidOffsets.size() - 1, Vector3());

		NvFlexSetRigids(
			pFlexSolver,
			&RigidOffsets[0],
			&RigidIndices[0],
			(float*)rigidLocalPositions.data(),
			nullptr,
			RigidCoefficients.data(),
			(float*)&RigidRotations[0],
			(float*)&RigidTranslations[0],
			numRigids
		);
	}

	//Springs
	if (SpringIndices.size() > 0)
		NvFlexSetSprings(pFlexSolver, SpringIndices.data(), SpringLengths.data(), SpringStiffness.data(), SpringLengths.size());

	//Convex shapes
	if (ShapePositions.size() > 0)
	{
		NvFlexSetShapes(pFlexSolver,
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
		              ShapeStarts.size()
		);
	}

	NvFlexSetPhases(pFlexSolver, Phases.data(), numParticles);

	RestPositions.insert(RestPositions.begin(), Positions.begin(), Positions.end());
	NvFlexSetRestParticles(pFlexSolver, (float*)RestPositions.data(), numParticles);

	Console::LogFormat(LogType::INFO, "FlexSystem::UploadFlexData() > Flex data uploaded successfully: %i particles", Positions.size());
}

void FlexSystem::SetDefaultParams()
{
	Params.radius = 0.15f;

	Params.gravity[0] = 0.0f;
	Params.gravity[1] = -9.8f;
	Params.gravity[2] = 0.0f;

	Params.wind[0] = 0.0f;
	Params.wind[1] = 0.0f;
	Params.wind[2] = 0.0f;

	Params.viscosity = 0.0f;
	Params.dynamicFriction = 0.0f;
	Params.staticFriction = 0.0f;
	Params.particleFriction = 0.0f; // scale friction between particles by default
	Params.freeSurfaceDrag = 0.0f;
	Params.drag = 0.0f;
	Params.lift = 0.0f;
	Params.numIterations = 3;
	Params.fluidRestDistance = 0.0f;
	Params.solidRestDistance = 0.0f;
	Params.anisotropyScale = 1.0f;
	Params.anisotropyMin = 0.1f;
	Params.anisotropyMax = 2.0f;
	Params.smoothing = 1.0f;
	Params.dissipation = 0.0f;
	Params.damping = 0.0f;
	Params.particleCollisionMargin = 0.0f;
	Params.shapeCollisionMargin = 0.0f;
	Params.collisionDistance = 0.0f;
	Params.plasticThreshold = 0.0f;
	Params.plasticCreep = 0.0f;
	Params.fluid = false;
	Params.sleepThreshold = 0.0f;
	Params.shockPropagation = 0.0f;
	Params.restitution = 0.0f;
	Params.maxSpeed = FLT_MAX;
	Params.relaxationMode = NvFlexRelaxationMode::eNvFlexRelaxationLocal;
	Params.relaxationFactor = 1.0f;
	Params.solidPressure = 1.0f;
	Params.adhesion = 0.0f;
	Params.cohesion = 0.025f;
	Params.surfaceTension = 0.0f;
	Params.vorticityConfinement = 0.0f;
	Params.buoyancy = 1.0f;

	Params.diffuseThreshold = 100.0f;
	Params.diffuseBuoyancy = 1.0f;
	Params.diffuseDrag = 0.8f;
	Params.diffuseBallistic = 16;
	Params.diffuseSortAxis[0] = 0.0f;
	Params.diffuseSortAxis[1] = 0.0f;
	Params.diffuseSortAxis[2] = 0.0f;
	Params.diffuseLifetime = 2.0f;

	Params.numPlanes = 0;
}

void FlexSystem::CreateGroundPlane(const Vector3& normal, const float distance)
{
	int planeIdx = Params.numPlanes;
	++Params.numPlanes;
	reinterpret_cast<Vector4&>(Params.planes[planeIdx]) = Vector4(normal.x, normal.y, normal.z, -distance);
}

void FlexSystem::AdjustParams()
{
	// by default solid particles use the maximum radius
	if (Params.fluid && Params.solidRestDistance == 0.0f)
		Params.solidRestDistance = Params.fluidRestDistance;
	else
		Params.solidRestDistance = Params.radius;
	// collision distance with shapes half the radius
	if (Params.collisionDistance == 0.0f)
	{
		Params.collisionDistance = Params.radius * 0.5f;

		if (Params.fluid)
			Params.collisionDistance = Params.fluidRestDistance * 0.5f;
	}
	// default particle friction to 10% of shape friction
	if (Params.particleFriction == 0.0f)
		Params.particleFriction = Params.dynamicFriction * 0.1f;
	// add a margin for detecting contacts between particles and shapes
	if (Params.shapeCollisionMargin == 0.0f)
		Params.shapeCollisionMargin = Params.collisionDistance * 0.5f;
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
			int rigidIndex = RigidIndices[i];
			average = average + Vector3((float*)&Positions[rigidIndex]);
		}
		average = average / float(clusterSize);
		for (int i = startIndex; i < endIndex; ++i)
		{
			int rigidIdx = RigidIndices[i];
			//Subtract the average position from the particle position to get the local position
			localPositions[count++] = Vector3((float*)&Positions[rigidIdx]) - average;
		}
	}
}