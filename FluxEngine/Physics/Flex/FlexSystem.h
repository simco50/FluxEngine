#pragma once
class MeshFilter;

class FlexSystem
{
public:
	FlexSystem();
	~FlexSystem();

	void InitializeSolver();

	void FetchData();

	void UpdateSolver(float deltaTime = 1.0f / 60.0f);

	void UpdateData();

	FlexMemory MemoryType = eFlexMemoryHost;
	FlexSolver* pFlexSolver = nullptr;

	FlexParams Params;
	int Substeps = 3;

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

	//Collision Geometry
	vector<int> ShapeStarts;
	vector<Vector4> ShapeAabbMin;
	vector<Vector4> ShapeAabbMax;
	vector<Vector4> ShapePositions;
	vector<Vector4> ShapeRotations;
	vector<Vector4> ShapePrevPositions;
	vector<Vector4> ShapePrevRotations;
	vector<FlexCollisionGeometry> ShapeGeometry;
	vector<int> ShapeFlags;

	//Methods
	/**
	* param[out]  Params
	* Default radius is 0.15f
	* Default earth gravity (-9.81f)
	*/
	void SetDefaultParams();

	/**
	* Adds a plane to the Params
	* param[out] params Reference to the Params
	* param[in] normal The normal of the plane
	* param[in] distance The distance of the plane (in the normal direction)
	*/
	void CreateGroundPlane(const Vector3& normal = Vector3(0, 1, 0), const float distance = 0.0f);

	void AdjustParams();

	/** Transforms all the particles relative to the space of their cluster center (one particle can be part of multiple clusters)
	* [in]param flexData The positions of the particles (4*n)s
	* [in]param numRigids The amount of rigids
	* [out]param localPositions
	*/
	void CalculateRigidOffsets(const int numRigids, Vector3* localPositions);

	void UploadFlexData();
};