// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2013-2015 NVIDIA Corporation. All rights reserved.


#ifndef FLEX_H
#define FLEX_H

//! \cond HIDDEN_SYMBOLS
#if _WIN32
#define FLEX_API __declspec(dllexport)
#else
#define FLEX_API
#endif

// least 2 significant digits define minor version, eg: 10 -> version 0.10
#define FLEX_VERSION 100

//! \endcond

extern "C" {

/**
 * Opaque type representing a collection of particles and constraints
 */
struct FlexSolver;

/**
 * Simulation parameters for a solver
 */
enum FlexRelaxationMode
{
	eFlexRelaxationGlobal = 0,			//!< The relaxation factor is a fixed multiplier on each constraint's position delta
	eFlexRelaxationLocal  = 1			//!< The relaxation factor is a fixed multiplier on each constraint's delta divided by the particle's constraint count, convergence will be slower but more reliable
};

struct FlexParams
{
	int mNumIterations;					//!< Number of solver iterations to perform per-substep

	float mGravity[3];					//!< Constant acceleration applied to all particles
	float mRadius;						//!< The maximum interaction radius for particles
	float mSolidRestDistance;			//!< The distance non-fluid particles attempt to maintain from each other, must be in the range (0, radius]
	float mFluidRestDistance;			//!< The distance fluid particles are spaced at the rest density, must be in the range (0, radius], for fluids this should generally be 50-70% of mRadius, for rigids this can simply be the same as the particle radius

	// common params
	float mDynamicFriction;				//!< Coefficient of friction used when colliding against shapes
	float mStaticFriction;				//!< Coefficient of static friction used when colliding against shapes
	float mParticleFriction;			//!< Coefficient of friction used when colliding particles
	float mRestitution;					//!< Coefficient of restitution used when colliding against shapes, particle collisions are always inelastic
	float mAdhesion;					//!< Controls how strongly particles stick to surfaces they hit, default 0.0, range [0.0, +inf]
	float mSleepThreshold;				//!< Particles with a velocity magnitude < this threshold will be considered fixed
	float mMaxSpeed;					//!< The magnitude of particle velocity will be clamped to this value at the end of each step
	float mShockPropagation;			//!< Artificially decrease the mass of particles based on height from a fixed reference point, this makes stacks and piles converge faster
	float mDissipation;					//!< Damps particle velocity based on how many particle contacts it has
	float mDamping;						//!< Viscous drag force, applies a force proportional, and opposite to the particle velocity
	float mInertiaBias;					//!< Increase the amount of inertia in shape-matching clusters to improve stability

	// cloth params
	float mWind[3];						//!< Constant acceleration applied to particles that belong to dynamic triangles, drag needs to be > 0 for wind to affect triangles
	float mDrag;						//!< Drag force applied to particles belonging to dynamic triangles, proportional to velocity^2*area in the negative velocity direction
	float mLift;						//!< Lift force applied to particles belonging to dynamic triangles, proportional to velocity^2*area in the direction perpendicular to velocity and (if possible), parallel to the plane normal

	// fluid params
	bool mFluid;						//!< If true then particles with phase 0 are considered fluid particles and interact using the position based fluids method
	float mCohesion;					//!< Control how strongly particles hold each other together, default: 0.025, range [0.0, +inf]
	float mSurfaceTension;				//!< Controls how strongly particles attempt to minimize surface area, default: 0.0, range: [0.0, +inf]    
	float mViscosity;					//!< Smoothes particle velocities using XSPH viscosity
	float mVorticityConfinement;		//!< Increases vorticity by applying rotational forces to particles
	float mAnisotropyScale;				//!< Control how much anisotropy is present in resulting ellipsoids for rendering, if zero then anisotropy will not be calculated, see flexGetAnisotropy()
	float mAnisotropyMin;				//!< Clamp the anisotropy scale to this fraction of the radius
	float mAnisotropyMax;				//!< Clamp the anisotropy scale to this fraction of the radius
	float mSmoothing;					//!< Control the strength of Laplacian smoothing in particles for rendering, if zero then smoothed positions will not be calculated, see flexGetSmoothParticles()
	float mSolidPressure;				//!< Add pressure from solid surfaces to particles
	float mFreeSurfaceDrag;				//!< Drag force applied to boundary fluid particles
	float mBuoyancy;					//!< Gravity is scaled by this value for fluid particles

	// diffuse params
	float mDiffuseThreshold;			//!< Particles with kinetic energy + divergence above this threshold will spawn new diffuse particles
	float mDiffuseBuoyancy;				//!< Scales force opposing gravity that diffuse particles receive
	float mDiffuseDrag;					//!< Scales force diffuse particles receive in direction of neighbor fluid particles
	int mDiffuseBallistic;				//!< The number of neighbors below which a diffuse particle is considered ballistic
	float mDiffuseSortAxis[3];			//!< Diffuse particles will be sorted by depth along this axis if non-zero
	float mDiffuseLifetime;				//!< Time in seconds that a diffuse particle will live for after being spawned, particles will be spawned with a random lifetime in the range [0, mDiffuseLifetime]

	// rigid params
	float mPlasticThreshold;			//!< Particles belonging to rigid shapes that move with a position delta magnitude > threshold will be permanently deformed in the rest pose
	float mPlasticCreep;				//!< Controls the rate at which particles in the rest pose are deformed for particles passing the deformation threshold 

	// collision params
	float mCollisionDistance;			//!< Distance particles maintain against shapes, note that for robust collision against triangle meshes this distance should be greater than zero
	float mParticleCollisionMargin;		//!< Increases the radius used during neighbor finding, this is useful if particles are expected to move significantly during a single step to ensure contacts aren't missed on subsequent iterations
	float mShapeCollisionMargin;		//!< Increases the radius used during contact finding against kinematic shapes

	float mPlanes[8][4];				//!< Collision planes in the form ax + by + cz + d = 0
	int mNumPlanes;						//!< Num collision planes

	FlexRelaxationMode mRelaxationMode;	//!< How the relaxation is applied inside the solver
	float mRelaxationFactor;			//!< Control the convergence rate of the parallel solver, default: 1, values greater than 1 may lead to instability
};

/**
 * Flags that control the a particle's behavior and grouping
 */
enum FlexPhase
{
	eFlexPhaseGroupMask			= 0x00ffffff,	//!< Low 24 bits represent the particle group for controlling collisions	

	eFlexPhaseSelfCollide		= 1 << 24,		//!< If set this particle will interact with particles of the same group
	eFlexPhaseSelfCollideFilter = 1 << 25,		//!< If set this particle will ignore collisions with particles closer than the radius in the rest pose, this flag should not be specified unless valid rest positions have been specified using flexSetRestParticles()
	eFlexPhaseFluid				= 1 << 26,		//!< If set this particle will generate fluid density constraints for its overlapping neighbors
};

/**
 * Generate a bit set for the particle phase, the group should be an integer < 2^24, and the flags should be a combination of FlexPhase enum values
 */
FLEX_API inline int flexMakePhase(int group, int flags) { return (group & eFlexPhaseGroupMask) | flags; }


/**
 * Time spent in each section of the solver update, times in seconds, see flexUpdateSolver()
 */
struct FlexTimers
{
	float mPredict;				//!< Time spent in prediction
	float mCreateCellIndices;	//!< Time spent creating grid indices
	float mSortCellIndices;		//!< Time spent sorting grid indices
	float mCreateGrid;			//!< Time spent creating grid
	float mReorder;				//!< Time spent reordering particles
	float mCollideParticles;	//!< Time spent finding particle neighbors
	float mCollideShapes;		//!< Time spent colliding convex shapes
	float mCollideTriangles;	//!< Time spent colliding triangle shapes
	float mCollideFields;		//!< Time spent colliding signed distance field shapes
	float mCalculateDensity;	//!< Time spent calculating fluid density
	float mSolveDensities;		//!< Time spent solving density constraints
	float mSolveVelocities;		//!< Time spent solving velocity constraints
	float mSolveShapes;			//!< Time spent solving rigid body constraints
	float mSolveSprings;		//!< Time spent solving distance constraints
	float mSolveContacts;		//!< Time spent solving contact constraints
	float mSolveInflatables;	//!< Time spent solving pressure constraints
	float mCalculateAnisotropy;	//!< Time spent calculating particle anisotropy for fluid
	float mUpdateDiffuse;		//!< Time spent updating diffuse particles
	float mUpdateTriangles;		//!< Time spent updating dynamic triangles
	float mUpdateNormals;		//!< Time spent updating vertex normals
	float mFinalize;			//!< Time spent finalizing state
	float mUpdateBounds;		//!< Time spent updating particle bounds
	float mTotal;				//!< Sum of all timers above
};

/**
 * Flex error return codes
 */
enum FlexError
{
	//! The API call returned with no errors. 
	eFlexErrorNone				= 0,

	//! The header version does not match the library binary
	eFlexErrorWrongVersion		= 1,

	//! The GPU associated with the calling thread does not meet
	//! requirements. An SM3.0 GPU or above is required
	eFlexErrorInsufficientGPU	= 2,

	//! Could not initialize CUDA context
	eFlexErrorDriverFailure		= 3
};

/**
 * Designates a memory space for getting/setting data to/from
 */
enum FlexMemory
{
	//! Host (CPU) memory
	eFlexMemoryHost			= 0,

	//! Device (GPU) memory
	eFlexMemoryDevice		= 1,

	//! Host (CPU) memory asynchronous, when used
	//! with a flexGet/flexSet method the memory
	//! transfer will be asynchronous and should be
	//! synchronized with flexWaitFence()
	eFlexMemoryHostAsync	= 2,

	//! Device (GPU) memory asynchronous, when used
	//! with a flexGet/flexSet method the memory
	//! transfer will be asynchronous and should be
	//! synchronized with flexWaitFence()
	eFlexMemoryDeviceAsync	= 3
};
 
/** Defines the set of stages at which callbacks may be registered 
*/
enum FlexSolverCallbackStage
{
	eFlexStageIterationStart,	//!< Called at the beginning of each constraint iteration
	eFlexStageIterationEnd,		//!< Called at the end of each constraint iteration
	eFlexStageSubstepBegin,		//!< Called at the beginning of each substep after the prediction step has been completed
	eFlexStageSubstepEnd,		//!< Called at the end of each substep after the velocity has been updated by the constraints
	eFlexStageUpdateEnd,		//!< Called at the end of solver update after the final substep has completed
	eFlexStageCount,			//!< Number of stages
};

/** Structure containing pointers to the internal solver data that is passed to each registered solver callback
 *
 *  @remarks Pointers to internal data are only valid for the lifetime of the callback and should not be stored.
 *  However, it is safe to launch kernels and memory transfers using the device pointers.
 *
 *  @remarks Because Flex re-orders particle data internally for performance, the particle data in the callback is not
 *  in the same order as it was provided to the API. The callback provides arrays which map original particle indices
 *  to sorted positions and vice-versa.
 *
 *  @remarks Particle positions may be modified during any callback, but velocity modifications should only occur during 
 *  the eFlexStageUpdateEnd stage, otherwise any velocity changes will be discarded.
 */
struct FlexSolverCallbackParams
{
	FlexSolver* mSolver;				//!< Pointer to the solver that the callback is registered to
	void* mUserData;					//!< Pointer to the user data provided to flexRegisterSolverCallback()

	float* mParticles;					//!< Device pointer to the active particle basic data in the form x,y,z,1/m
	float* mVelocities;					//!< Device pointer to the active particle velocity data in the form x,y,z,w (last component is not used)
	int* mPhases;						//!< Device pointer to the active particle phase data

	int mNumActive;						//!< The number of active particles returned, the callback data only return pointers to active particle data, this is the same as flexGetActiveCount()
	
	float mDt;							//!< The per-update time-step, this is the value passed to flexUpdateSolver()

	const int* mOriginalToSortedMap;	//!< Device pointer that maps the sorted callback data to the original position given by flexSetParticles()
	const int* mSortedToOriginalMap;	//!< Device pointer that maps the original particle index to the index in the callback data structure
};

/** Solver callback definition, see flexRegisterSolverCallback()
 */
struct FlexSolverCallback
{
	/** User data passed to the callback*/
	void* mUserData;
	
	/** Function pointer to a callback method */
	void (*mFunction)(FlexSolverCallbackParams params);
};

/**
 * Types of log severity
 */
enum FlexErrorSeverity
{
	eFlexLogError	=  0,	//!< Error messages
	eFlexLogInfo	=  1,	//!< Information messages
	eFlexLogWarning	=  2,	//!< Warning messages
	eFlexLogDebug	=  4,	//!< Used only in debug version of dll
	eFlexLogAll		= -1,	//!< All log types
};

/**
 * Function pointer type for error reporting callbacks
 */
typedef void (*FlexErrorCallback)(FlexErrorSeverity type, const char* msg, const char* file, int line);

/**
 * Initialize library, should be called before any other API function.
 *
 *
 * @param[in] version The version number the app is expecting, should almost always be FLEX_VERSION
 * @param[in] errorFunc The callback used for reporting errors.
 * @param[in] deviceIndex The CUDA device index that should be used, if there is already a CUDA context on the calling thread then this parameter will be ignored and the active CUDA context used. 
						  If the deviceIndex is -1 then the device will be set according to the PhysX control panel recommended device.
						  Otherwise a new context will be created using the suggested device ordinal.
 */
FLEX_API FlexError flexInit(int version=FLEX_VERSION, FlexErrorCallback errorFunc=0, int deviceIndex=-1);

/**
 * Shutdown library, users should manually destroy any previously created   
 * solvers to ensure memory is freed before calling this method. If a new CUDA context was created during flexInit() then it will be destroyed.
 */
FLEX_API void flexShutdown();

/**
 * Get library version number
 */
FLEX_API int flexGetVersion();

/**
 * Create a new particle solver
 *
 * @param[in] maxParticles Maximum number of simulation particles possible for this solver
 * @param[in] maxDiffuseParticles Maximum number of diffuse (non-simulation) particles possible for this solver
 * @param[in] maxNeighborsPerParticle Maximum number of neighbors per particle possible for this solver
 */
FLEX_API FlexSolver* flexCreateSolver(int maxParticles, int maxDiffuseParticles, unsigned char maxNeighborsPerParticle = 96);
/**
 * Delete a particle solver
 */
FLEX_API void flexDestroySolver(FlexSolver* s);

/** Registers a callback for a solver stage, the callback will be invoked from the same thread that calls flexUpdateSolver().
 *
 * @param[in] s A valid solver
 * @param[in] function A pointer to a function that will be called during the solver update
 * @param[in] stage The stage of the update at which the callback function will be called
 *
 * @return The previously registered callback for this slot, this allows multiple users to chain callbacks together
 */
FLEX_API FlexSolverCallback flexRegisterSolverCallback(FlexSolver* s, FlexSolverCallback function, FlexSolverCallbackStage stage);

/**
 * Move particle solver forward in time
 *
 * @param[in] s A valid solver
 * @param[in] dt Time to integrate the solver forward in time by
 * @param[in] substeps The time dt will be divided into the number of sub-steps given by this parameter
 * @param[out] timers If non-NULL this struct will be filled out with profiling information for the step, note that profiling can substantially slow down overal performance so this param should only be non-NULL in non-release builds
 */
FLEX_API void flexUpdateSolver(FlexSolver* s, float dt, int substeps, FlexTimers* timers);

/**
 * Update solver paramters
 *
 * @param[in] s A valid solver
 * @param[in] params Parameters structure in host memory, see FlexParams
 */
FLEX_API void flexSetParams(FlexSolver* s, const FlexParams* params);

/**
 * Retrieve solver paramters, default values will be set at solver creation time
 *
 * @param[in] s A valid solver
 * @param[out] params Parameters structure in host memory, see FlexParams
 */

FLEX_API void flexGetParams(FlexSolver* s, FlexParams* params);

/**
 * Set the active particles indices in the solver
 * 
 * @param[in] s A valid solver
 * @param[in] indices Holds the indices of particles that have been made active
 * @param[in] n Number of particles to allocate
 * @param[in] source The memory space of the indices
 */
FLEX_API void flexSetActive(FlexSolver* s, const int* indices, int n, FlexMemory source);

/**
 * Return the active particle indices
 * 
 * @param[in] s A valid solver
 * @param[out] indices An array of indices at least activeCount in length
 * @param[in] target The memory space of the destination buffer
 */
FLEX_API void flexGetActive(FlexSolver* s, int* indices, FlexMemory target);

/**
 * Return the number of active particles in the solver
 * 
 * @param[in] s A valid solver
 * @return The number of active particles in the solver
 */
FLEX_API int flexGetActiveCount(FlexSolver* s);

/**
 * Set the particles state of the solver, a particle consists of 4 floating point numbers, its x,y,z position followed by its inverse mass (1/m)
 * 
 * @param[in] s A valid solver
 * @param[in] p Pointer to an array of particle data, should be 4*n in length
 * @param[in] n The number of particles to set
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetParticles(FlexSolver* s, const float* p, int n, FlexMemory source);

/**
 * Get the particles state of the solver, a particle consists of 4 floating point numbers, its x,y,z position followed by its inverse mass (1/m)
 * 
 * @param[in] s A valid solver
 * @param[out] p Pointer to an array of 4*n floats that will be filled out with the particle data, can be either a host or device pointer
 * @param[in] n The number of particles to get, must be less than max particles passed to flexCreateSolver
 * @param[in] target The memory space of the destination buffer
 */
FLEX_API void flexGetParticles(FlexSolver* s, float* p, int n, FlexMemory target);

/**
 * Set the particle positions in their rest state, if FlexPhase::eFlexPhaseSelfCollideFilter is set on the particle's
 * phase attribute then particles that overlap in the rest state will not generate collisions with each other
 * 
 * @param[in] s A valid solver
 * @param[in] p Pointer to an array of particle data, should be 4*n in length
 * @param[in] n The number of particles to set
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetRestParticles(FlexSolver* s, const float* p, int n, FlexMemory source);

/**
 * Get the Laplacian smoothed particle positions for rendering, see FlexParams::mSmoothing
 * 
 * @param[in] s A valid solver
 * @param[out] p Pointer to an array of 4*n floats that will be filled out with the data, can be either a host or device pointer
 * @param[in] n The number of smooth particles to return
 * @param[in] target The memory space of the destination buffer
 */
FLEX_API void flexGetSmoothParticles(FlexSolver* s, float* p, int n, FlexMemory target);

/**
 * Set the particle velocities, each velocity is a 3-tuple of x,y,z floating point values
 * 
 * @param[in] s A valid solver
 * @param[in] v Pointer to an array of 3*n floats
 * @param[in] n The number of velocities to set 
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetVelocities(FlexSolver* s, const float* v, int n, FlexMemory source);
/**
 * Get the particle velocities, each velocity is a 3-tuple of x,y,z floating point values
 * 
 * @param[in] s A valid solver
 * @param[out] v Pointer to an array of 3*n floats that will be filled out with the data, can be either a host or device pointer
 * @param[in] n The number of velocities to get
 * @param[in] target The memory space of the destination buffer
 */
FLEX_API void flexGetVelocities(FlexSolver* s, float* v, int n, FlexMemory target);

/**
 * Set the particles phase id array, each particle has an associated phase id which 
 * controls how it interacts with other particles. Particles with phase 0 interact with all
 * other phase types.
 *
 * Particles with a non-zero phase id only interact with particles whose phase differs 
 * from theirs. This is useful, for example, to stop particles belonging to a single
 * rigid shape from interacting with each other.
 * 
 * Phase 0 is used to indicate fluid particles when FlexParams::mFluid is set.
 * 
 * @param[in] s A valid solver
 * @param[in] phases Pointer to an array of n integers containing the phases
 * @param[in] n The number of phases to set
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetPhases(FlexSolver* s, const int* phases, int n, FlexMemory source);
/**
 * Get the particle phase ids
 * 
 * @param[in] s A valid solver
 * @param[out] phases Pointer to an array of n integers that will be filled with the phase data, can be either a host or device pointer
 * @param[in] n The number of phases to get
 * @param[in] target The memory space of the destination buffer
 */
FLEX_API void flexGetPhases(FlexSolver* s, int* phases, int n, FlexMemory target);

/**
 * Set distance constraints for the solver. Each distance constraint consists of two particle indices
 * stored consecutively, a rest-length, and a stiffness value. These are not springs in the traditional
 * sense, but behave somewhat like a traditional spring when lowering the stiffness coefficient.
 * 
 * @param[in] s A valid solver
 * @param[in] indices Pointer to the spring indices array, should be 2*numSprings length, 2 indices per-spring
 * @param[in] restLengths Pointer to an array of rest lengths, should be numSprings length
 * @param[in] stiffness Pointer to the spring stiffness coefficents, should be numSprings in length, a negative stiffness value represents a tether constraint
 * @param[in] numSprings The number of springs to set
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetSprings(FlexSolver* s, const int* indices, const float* restLengths, const float* stiffness, int numSprings, FlexMemory source);
/**
 * Get the distance constraints from the solver
 * 
 * @param[in] s A valid solver
 * @param[out] indices Pointer to the spring indices array, should be 2*numSprings length, 2 indices per-spring
 * @param[out] restLengths Pointer to an array of rest lengths, should be numSprings length
 * @param[out] stiffness Pointer to the spring stiffness coefficents, should be numSprings in length, a negative stiffness value represents a unilateral tether constraint (only resists stretching, not compression), valid range [-1, 1]
 * @param[in] numSprings The number of springs to get
 * @param[in] target The memory space of the destination buffers
 */
FLEX_API void flexGetSprings(FlexSolver* s, int* indices, float* restLengths, float* stiffness, int numSprings, FlexMemory target);

/**
 * Set rigid body constraints for the solver. 
 * @note A particle should not belong to more than one rigid body at a time.
 * 
 * @param[in] s A valid solver
 * @param[in] offsets Pointer to an array of start offsets for a rigid in the indices array, should be numRigids+1 in length, the first entry must be 0
 * @param[in] indices Pointer to an array of indices for the rigid bodies, the indices for the jth rigid body start at indices[offsets[j]] and run to indices[offsets[j+1]] exclusive
 * @param[in] restPositions Pointer to an array of local space positions relative to the rigid's center of mass (average position), this should be at least 3*numIndices in length in the format x,y,z
 * @param[in] restNormals Pointer to an array of local space normals, this should be at least 4*numIndices in length in the format x,y,z,w where w is the (negative) signed distance of the particle inside its shape
 * @param[in] stiffness Pointer to an array of rigid stiffness coefficents, should be numRigids in length, valid values in range [0, 1]
 * @param[in] rotations Pointer to an array of quaternions (4*numRigids in length)
 * @param[in] translations Pointer to an array of translations of the center of mass (3*numRigids in length)
 * @param[in] numRigids The number of rigid bodies to set
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetRigids(FlexSolver* s, const int* offsets, const int* indices, const float* restPositions, const float* restNormals, const float* stiffness, const float* rotations, const float* translations, int numRigids, FlexMemory source);

/**
 * Set per-particle normals to the solver, these will be overwritten after each simulation step, but can be used to initialize the normals to valid values
 * 
 * @param[in] s A valid solver
 * @param[in] normals Pointer to an array of normals, should be 4*n in length
 * @param[in] n The number of normals to set
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetNormals(FlexSolver* s, const float* normals, int n, FlexMemory source);

/**
 * Get per-particle normals from the solver, these are the world-space normals computed during surface tension, cloth, and rigid body calculations
 * 
 * @param[in] s A valid solver
 * @param[out] normals Pointer to an array of normals, should be 4*n in length
 * @param[in] n The number of normals to get
 * @param[in] target The memory space of the destination buffer
 */
FLEX_API void flexGetNormals(FlexSolver* s, float* normals, int n, FlexMemory target);

/**
 * Get the rotation matrices for the rigid bodies in the solver
 * 
 * @param[in] s A valid solver
 * @param[out] rotations Pointer to an array of quaternions, should be 4*numRigids floats in length
 * @param[out] translations Pointer to an array of vectors to hold the rigid translations, should be 3*numRigids floats in length
 * @param[in] target The memory space of the destination buffer
 */
FLEX_API void flexGetRigidTransforms(FlexSolver* s, float* rotations, float* translations, FlexMemory target);

/**
 * An opaque type representing a static triangle mesh in the solver
 */
struct FlexTriangleMesh;

/**
 * Create triangle mesh geometry, note that meshes may be used by multiple solvers if desired
 * 
 * @return A pointer to a triangle mesh object
 */
FLEX_API FlexTriangleMesh* flexCreateTriangleMesh();

/**
 * Destroy a triangle mesh created with flexCreateTriangleMesh()
 *
 * @param[in] mesh A triangle mesh created with flexCreateTriangleMesh()
 */
FLEX_API void flexDestroyTriangleMesh(FlexTriangleMesh* mesh);

/**
 * Specifies the triangle mesh geometry (vertices and indices), this method will cause any internal
 * data structures (e.g.: bounding volume hierarchies) to be rebuilt.
 *
 * @param[in] mesh A triangle mesh created with flexCreateTriangleMesh()
 * @param[in] vertices Pointer to an array of float3 vertex positions
 * @param[in] indices Pointer to an array of triangle indices, should be length numTriangles*3
 * @param[in] numVertices The number of vertices in the vertices array
 * @param[in] numTriangles The number of triangles in the mesh
 * @param[in] lower A pointer to a float3 vector holding the lower spatial bounds of the mesh
 * @param[in] upper A pointer to a float3 vector holding the upper spatial bounds of the mesh
 * @param[in] source The memory space the transfers to the GPU will occur from
 */
FLEX_API void flexUpdateTriangleMesh(FlexTriangleMesh* mesh, const float* vertices, const int* indices, int numVertices, int numTriangles, const float* lower, const float* upper, FlexMemory source);

/**
 * Retrive the local space bounds of the mesh, these are the same values specified to flexUpdateTriangleMesh()
 * 
 * @param[in] mesh Pointer to a triangle mesh object
 * @param[out] lower Pointer to an array of 3 floats that the lower mesh bounds will be written to 
 * @param[out] upper Pointer to an array of 3 floats that the upper mesh bounds will be written to 
 */
FLEX_API void flexGetTriangleMeshBounds(const FlexTriangleMesh* mesh, float* lower, float* upper);

/**
 * An opaque type representing a signed distance field collision shape in the solver
 */
struct FlexSDF;

/**
 * Set the signed distance field collision shapes, see FlexSDF for details.
 * 
 * @return A pointer to a signed distance field object
 */
FLEX_API FlexSDF* flexCreateSDF();

/**
 * Destroy a signed distance field
 *
 * @param[in] sdf A signed distance field created with flexCreateSDF()
 */
FLEX_API void flexDestroySDF(FlexSDF* sdf);

/**
 * Update the signed distance field volume data, this method will upload
 * the field data to a 3D texture on the GPU
 *
 * @param[in] sdf A signed distance field created with flexCreateSDF()
 * @param[in] dimx The x-dimension of the volume data in voxels
 * @param[in] dimy The y-dimension of the volume data in voxels
 * @param[in] dimz The z-dimension of the volume data in voxels
 * @param[in] field The volume data stored such that the voxel at the x,y,z coordinate is addressed as field[z*dimx*dimy + y*dimx + x]
 * @param[in] source The memory space the transfers to the GPU will occur from
 */
 FLEX_API void flexUpdateSDF(FlexSDF* sdf, int dimx, int dimy, int dimz, const float* field, FlexMemory source);

/**
 * A basic sphere shape with origin at the center of the sphere and radius
 */
struct FlexCollisionSphere
{
	float mRadius;
};

/**
 * A collision capsule extends along the x-axis with it's local origin at the center of the capsule 
 */
struct FlexCollisionCapsule
{
	float mRadius;
	float mHalfHeight;
};

/**
 * A collision plane is an infinite half-space represented as a plane equation with the form a*x + b*y + c*z + d = 0. 
 * Particles are constrained to the positive half space. Multiple planes can be added to the geometry array to form a convex mesh. 	
 */
struct FlexCollisionPlane
{	
	float mPlane[4];
};

/** 
 * A scaled triangle mesh 
 */
struct FlexCollisionTriangleMesh
{
	FlexTriangleMesh* mMesh;	//!< A triangle mesh pointer created by flexCreateTriangleMesh()
	float mScale;				//!< The scale of the object from local space to world space
};

/**
 * A scaled signed distance field 
 */
struct FlexCollisionSDF
{
	FlexSDF* mField;	//!< A signed distance field pointer created by flexCreateSDF()
	float mScale;		//!< Because field distances are normalized in the range from [0-1] in local space, the scale represents the world space dimensions of the volume
};

/**
 * This union allows collision geometry to be sent to Flex as a flat array of 16-byte data structures,
 * the shape flags array specifies the type for each shape, see flexSetShapes().
 */
union FlexCollisionGeometry
{
	FlexCollisionSphere mSphere;
	FlexCollisionCapsule mCapsule;
	FlexCollisionPlane mPlane;
	FlexCollisionTriangleMesh mTriMesh;
	FlexCollisionSDF mSDF;
};

enum FlexCollisionShapeType
{
	eFlexShapeSphere		= 0,		//!< A sphere shape has a single FlexCollisionSphere entry in the geometry array
	eFlexShapeCapsule		= 1,		//!< A capsule shape has a single FlexCollisionCapsule entry in the geometry array
	eFlexShapeConvexMesh	= 2,		//!< A convex mesh shape is composed of multiple FlexCollisionPlane entries in the geometry array, the planes for a convex shape are given by the shapeOffsets i and i+1'th entries, e.g.: the entries corresponding to the range [shapeOffsets[i], shapeOffests[i+1]).
	eFlexShapeTriangleMesh	= 3,		//!< A triangle mesh shape created through flexCreateTriangleMesh()
	eFlexShapeSDF			= 4,		//!< A signed distance field shape created through flexCreateSDF()
};

enum FlexCollisionShapeFlags
{
	eFlexShapeFlagTypeMask	= 0x7,		//!< Lower 3 bits holds the type of the collision shape
	eFlexShapeFlagDynamic	= 8,		//!< Indicates the shape is dynamic and should have lower priority over static collision shapes
	eFlexShapeFlagTrigger	= 16,		//!< Indicates that the shape is a trigger volume, this means it will not perform any collision response, but will be reported in the contacts array (see flexGetContacts())

	eFlexShapeFlagReserved	= 0xffffff00
};

/** Combines geometry type and static dynamic 
 *
 */
FLEX_API inline int flexMakeShapeFlags(FlexCollisionShapeType type, bool dynamic) { return type | (dynamic?eFlexShapeFlagDynamic:0); }

/** DEPRECATED */
FLEX_API void flexSetFields(FlexSolver* s, const FlexSDF* shapes, int numShapes);


/**
 * Set the collision shapes for the solver
 * 
 * @param[in] s A valid solver
 * @param[in] geometry Pointer to an array of FlexCollisionGeometry entries, the type of each shape determines how many entries it has in the array
 * @param[in] numGeometryEntries The total number of geometry entries in the geometry array, spheres and capsules will each have one entry, while convex meshes will have multiple, one for each plane
 * @param[in] shapeAabbMins Pointer to an array of lower AABB coordinates for each shape in world space, should be 4*numShapes in length in x,y,z,* format
 * @param[in] shapeAabbMaxs Pointer to an array of upper AABB coordinates for each shape in world space, should be 4*numShapes in length in x,y,z,* format
 * @param[in] shapeOffsets Pointer to an array of offsets into the geometry array for each shape, should be numShapes in length
 * @param[in] shapePositions Pointer to an array of translations for each shape in world space, should be 4*numShapes in length
 * @param[in] shapeRotations Pointer to an an array of rotations for each shape stored as quaternion, should be 4*numShapes in length
 * @param[in] shapePrevPositions Pointer to an array of translations for each shape at the start of the time step, should be 4*numShapes in length
 * @param[in] shapePrevRotations Pointer to an an array of rotations for each shape stored as a quaternion at the start of the time step, should be 4*numShapees in length
 * @param[in] shapeFlags The type and behavior of the shape, FlexCollisionShapeFlags for more detail
 * @param[in] numShapes The number of shapes
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetShapes(FlexSolver* s, const FlexCollisionGeometry* geometry, int numGeometryEntries, const float* shapeAabbMins, const float* shapeAabbMaxs, const int* shapeOffsets, const float* shapePositions, const float* shapeRotations, const float* shapePrevPositions, const float* shapePrevRotations, const int* shapeFlags, int numShapes, FlexMemory source);

/**
 * Set dynamic triangles mesh indices, typically used for cloth. Flex will calculate normals and 
 * apply wind and drag effects to connected particles. See FlexParams::mDrag, FlexParams::mWind.
 * 
 * @param[in] s A valid solver
 * @param[in] indices Pointer to an array of triangle indices into the particles array, should be 3*numTris in length
 * @param[in] normals Pointer to an array of triangle normals, should be 3*numTris in length, can be NULL
 * @param[in] numTris The number of dynamic triangles
 * @param[in] source The memory space of the source buffers
 */
FLEX_API void flexSetDynamicTriangles(FlexSolver* s, const int* indices, const float* normals, int numTris, FlexMemory source);
/**
 * Get the dynamic triangle indices and normals.
  * 
 * @param[in] s A valid solver
 * @param[out] indices Pointer to an array of triangle indices into the particles array, should be 3*numTris in length, if NULL indices will not be returned
 * @param[out] normals Pointer to an array of triangle normals, should be 3*numTris in length, if NULL normals will be not be returned
 * @param[in] numTris The number of dynamic triangles
 * @param[in] target The memory space of the destination arrays
 */
FLEX_API void flexGetDynamicTriangles(FlexSolver* s, int* indices, float* normals, int numTris, FlexMemory target);

/**
 * Set inflatable shapes, an inflatable is a range of dynamic triangles (wound CCW) that represent a closed mesh.
 * Each inflatable has a given rest volume, constraint scale (roughly equivalent to stiffness), and "over pressure"
 * that controls how much the shape is inflated.
 * 
 * @param[in] s A valid solver
 * @param[in] startTris Pointer to an array of offsets into the solver's dynamic triangles for each inflatable, should be numInflatables in length
 * @param[in] numTris Pointer to an array of triangle counts for each inflatable, should be numInflatablesin length
 * @param[in] restVolumes Pointer to an array of rest volumes for the inflatables, should be numInflatables in length
 * @param[in] overPressures Pointer to an array of floats specifying the pressures for each inflatable, a value of 1.0 means the rest volume, > 1.0 means over-inflated, and < 1.0 means under-inflated, should be numInflatables in length
 * @param[in] constraintScales Pointer to an array of scaling factors for the constraint, this is roughly equivalent to stiffness but includes a constraint scaling factor from position-based dynamics, see helper code for details, should be numInflatables in length
 * @param[in] numInflatables Number of inflatables to set
 * @param[in] source The memory space of the source buffers
 */
FLEX_API void flexSetInflatables(FlexSolver* s, const int* startTris, const int* numTris, float* restVolumes, float* overPressures, float* constraintScales, int numInflatables, FlexMemory source);

/**
 * Get the density values for fluid particles
 * 
 * @param[in] s A valid solver
 * @param[out] densities Pointer to an array of floats, should be maxParticles in length, density values are normalized between [0, 1] where 1 represents the rest density
 * @param[in] target The memory space of the destination arrays
 */
FLEX_API void flexGetDensities(FlexSolver* s, float* densities, FlexMemory target);
/**
 * Get the anisotropy of fluid particles, the particle distribution for a particle is represented
 * by 3 orthogonal vectors. Each 3-vector has unit length with the variance along that axis
 * packed into the w component, i.e.: x,y,z,lambda.
 *
 * The anisotropy defines an oriented ellipsoid in worldspace that can be used for rendering
 * or surface extraction.
 * 
 * @param[in] s A valid solver
 * @param[out] q1 Pointer to an array of floats that receive the first basis vector and scale, should be 4*maxParticles in length
 * @param[out] q2 Pointer to an array of floats that receive the second basis vector and scale, should be 4*maxParticles in length
 * @param[out] q3 Pointer to an array of floats that receive the third basis vector and scale, should be 4*maxParticles in length
 * @param[in] target The memory space of the destination arrays
 */
FLEX_API void flexGetAnisotropy(FlexSolver* s, float* q1, float* q2, float* q3, FlexMemory target);
/**
 * Get the state of the diffuse particles. Diffuse particles are passively advected by the fluid
 * velocity field.
 * 
 * @param[in] s A valid solver
 * @param[out] p Pointer to an array of floats, should be 4*maxParticles in length, the w component represents the particles lifetime with 1 representing a new particle, and 0 representing an inactive particle
 * @param[out] v Pointer to an array of floats, should be 4*maxParticles in length, the w component is not used
 * @param[out] indices Pointer to an array of ints that specify particle indices in depth sorted order, should be maxParticles in length, see FlexParams::mDiffuseSortDir
 * @param[in] target The memory space of the destination arrays
 */
FLEX_API int flexGetDiffuseParticles(FlexSolver* s, float* p, float* v, int* indices, FlexMemory target);	
/**
 * Set the state of the diffuse particles. Diffuse particles are passively advected by the fluid
 * velocity field.
 * 
 * @param[in] s A valid solver
 * @param[in] p Pointer to an array of floats, should be 4*n in length, the w component represents the particles lifetime with 1 representing a new particle, and 0 representing an inactive particle
 * @param[in] v Pointer to an array of floats, should be 4*n in length, the w component is not used
 * @param[in] n Number of diffuse particles to set
 * @param[in] source The memory space of the source buffer
 */
FLEX_API void flexSetDiffuseParticles(FlexSolver* s, const float* p, const float* v, int n, FlexMemory source);

/**
 * Get the particle contact planes. Note this will only include contacts that were active on the last substep of an update, and will include all contact planes generated within FlexParam::mShapeCollisionMargin.
 * 
 * @param[in] s A valid solver
 * @param[out] planes Pointer to a destination buffer containing the contact planes for the particle, each particle can have up to 4 contact planes so this buffer should be 16*maxParticles in length
 * @param[out] velocities Pointer to a destination buffer containing the velocity of the contact point on the shape in world space, the index of the shape (corresponding to the shape in flexSetShapes() is stored in the w component), each particle can have up to 4 contact planes so this buffer should be 16*maxParticles in length
 * @param[out] indices Pointer to an array of indices into the contacts buffer, the first contact plane for the i'th particle is given by planes[indices[i]*sizeof(float)*4] and subsequent contacts for that particle are stored sequentially, this array should be maxParticles in length
 * @param[out] counts Pointer to an array of contact counts for each particle (will be <= 4), this buffer should be maxParticles in length
 * @param[in] target The memory space of the target buffers
 */
FLEX_API void flexGetContacts(FlexSolver* s, float* planes, float* velocities, int* indices, unsigned char* counts, FlexMemory target);

/**
 * Get the world space AABB of all particles in the solver, note that the bounds are calculated during the update (see flexUpdateSolver()) so only become valid after an update has been performed.
 * The returned bounds represent bounds of the particles in their predicted positions *before* the constraint solve.
 * 
 * @param[in] s A valid solver
 * @param[out] lower Pointer to an array of 3 floats to receive the lower bounds
 * @param[out] upper Pointer to an array of 3 floats to receive the upper bounds
 * @param[out] target The memory space of the target buffers, note that if an async transfer is requested then synchronization should be used to ensure transfer completion
 */
FLEX_API void flexGetBounds(FlexSolver* s, float* lower, float* upper, FlexMemory target);

/**
 * Allocates size bytes of memory from the optimal memory pool. Using this function
 * is optional, but when passed to flexGet/flexSet methods it may result
 * in significantly faster transfers, memory used with async transfers should
 * be allocated by this method to ensure optimal performance. For CUDA implementations
 * this method will return pinned host memory from cudaMallocHost().
 *
 @param[in] size The number of bytes to alloc
 @return pointer to the allocated memory
 */
FLEX_API void* flexAlloc(int size);

/** 
 * Free memory allocated through flexAlloc
 *
 * @param[in] ptr Pointer returned from flexAlloc
 */
FLEX_API void flexFree(void* ptr);

/** 
 * Ensures that the CUDA context the library was initialized with is present on the current thread
 */
FLEX_API void flexAcquireContext();

/** 
 * Restores the CUDA context (if any) that was present on the last call to flexAcquireContext()
 */
FLEX_API void flexRestoreContext();

/** 
 * Sets a fence that can be used to synchronize the calling thread 
 * with any outstanding GPU work, typically used with async transfers
 * to ensure any flexGet/flexSet calls have completed. 
 *
\code{.c}

	// update solver
	flexUpdateSolver(solver, dt, iterations, NULL);

	// read back state
	flexGetParticles(solver, &particles, n, eFlexMemoryHostAsync);
	flexGetVelocities(solver, &velocities, n, eFlexMemoryHostAsync);
	flexGetDensities(solver, &densities, n, eFlexMemoryHostAsync);

	// insert fence
	flexSetFence();

	// perform asynchronous CPU work
	
	// wait for queued work to finish
	flexWaitFence();

\endcode
 */
FLEX_API void flexSetFence();

/** 
 * Waits for the work scheduled before the last call to flexSetFence() to complete
 * If flexSetFence() has not yet been called then this is function returns immediately
 *
 */
FLEX_API void flexWaitFence();

//! \cond HIDDEN_SYMBOLS

/**
 * Debug methods (unsupported)
 */
FLEX_API void flexSetDebug(FlexSolver* s, bool enable);
FLEX_API void flexGetShapeGrid(FlexSolver* s, int* grid, float* lower, float* upper, int* axis);
FLEX_API void flexGetStaticTriangleGrid(FlexSolver* s, int* counts, float* lower, float* cellEdge);
FLEX_API void flexGetStaticTriangleBVH(FlexSolver* s, void* bvh);
FLEX_API void flexStartRecord(FlexSolver* s, const char* file);
FLEX_API void flexStopRecord(FlexSolver* s);

//! \endcond

} // extern "C"

#endif // FLEX_H

