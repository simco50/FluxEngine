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

#ifndef FLEX_EXT_H
#define FLEX_EXT_H

#include "../include/flex.h"

extern "C" {

/** 
 * Represents a group of particles and constraints, each asset 
 * can be instanced into a simulation using flexExtCreateInstance
 */
struct FlexExtAsset
{	
	// particles
	float* mParticles;				//!< Local space particle positions, x,y,z,1/mass
	int mNumParticles;				//!< Number of particles
	int mMaxParticles;				//!< Maximum number of particles

	// springs
	int* mSpringIndices;			//!< Spring indices
	float* mSpringCoefficients;		//!< Spring coefficients
	float* mSpringRestLengths;		//!< Spring rest-lengths
	int mNumSprings;				//!< Number of springs

	// shapes
	int* mShapeIndices;				//!< The indices of the shape matching constraints
	int mNumShapeIndices;			//!< Total number of indices for shape constraints	
	int* mShapeOffsets;				//!< Each entry stores the end of the shape's indices in the indices array (exclusive prefix sum of shape lengths)
	float* mShapeCoefficients;		//!< The stiffness coefficient for each shape
	float* mShapeCenters;			//!< The position of the center of mass of each shape, an array of vec3s mNumShapes in length
	int mNumShapes;					//!< The number of shape matching constraints

	// faces for cloth
	int* mTriangleIndices;			//!< Indexed triangle mesh for clothing
	int mNumTriangles;				//!< Number of triangles

	// inflatable params
	bool mInflatable;				//!< Whether an inflatable constraint should be added
	float mInflatableVolume;		//!< The rest volume for the inflatable constraint
	float mInflatablePressure;		//!< How much over the rest volume the inflatable should attempt to maintain
	float mInflatableStiffness;		//!< How stiff the inflatable is
};

/** 
 * Represents an instance of a FlexAsset in a container
 */
struct FlexExtInstance
{
	int* mParticleIndices;			//!< Simulation particle indices
	int mNumParticles;				//!< Number of simulation particles
	
	int mTriangleIndex;				//!< Index in the container's triangle array
	int mShapeIndex;				//!< Index in the container's shape body constraints array	
	int mInflatableIndex;			//!< Index in the container's inflatables array

	float* mShapeTranslations;		//!< Shape matching group translations (vec3s)
	float* mShapeRotations;			//!< Shape matching group rotations (quaternions)

	const FlexExtAsset* mAsset;		//!< Source asset used to create this instance
	
	void* mUserData;				//!< User data pointer
};

/** 
 * Controls the way that force fields affect particles
 */
enum FlexForceExtMode
{
	//! Apply field value as a force. 
    eFlexExtModeForce			=      0,

	//! Apply field value as an impulse. 
    eFlexExtModeImpulse			=      1,

	//! Apply field value as a velocity change. 
    eFlexExtModeVelocityChange	=      2,
};

/** 
 * Force field data, currently just supports radial fields
 */
struct FlexExtForceField
{
	float mPosition[3];		//!< Center of force field
	float mRadius;			//!< Radius of the force field
	float mStrength;		//!< Strength of the force field
	FlexForceExtMode mMode;	//!< Mode of field application
	bool mLinearFalloff;	//!< Linear or no falloff 
};

/** 
 * Opaque type representing a simulation
 */
struct FlexExtContainer;

/**
 * Create an index buffer of unique vertices in the mesh 
 *
 * @param[in] vertices A pointer to an array of float3 positions
 * @param[in] numVertices The number of vertices in the mesh
 * @param[out] uniqueVerts A list of unique mesh vertex indices, should be numVertices in length (worst case all verts are unique)
 * @param[out] originalToUniqueMap Mapping from the original vertex index to the unique vertex index, should be numVertices in length
 * @param[in] threshold The distance below which two vertices are considered duplicates
 * @return The number of unique vertices in the mesh
 */
FLEX_API int flexExtCreateWeldedMeshIndices(const float* vertices, int numVertices, int* uniqueVerts, int* originalToUniqueMap, float threshold);

/**
 * Create a cloth asset consisting of stretch and bend distance constraints given an indexed triangle mesh. Stretch constraints will be placed along
 * triangle edges, while bending constraints are placed over two edges.
 *
 * @param[in] particles Positions and masses of the particles in the format [x, y, z, 1/m]
 * @param[in] numParticles The number of particles
 * @param[in] indices The triangle indices, these should be 'welded' using flexExtCreateWeldedMeshIndices() first
 * @param[in] numTriangles The number of triangles
 * @param[in] stretchStiffness The stiffness coefficient for stretch constraints
 * @param[in] bendStiffness The stiffness coefficient used for bending constraints
 * @param[in] tetherStiffness If > 0.0f then the function will create tethers attached to particles with zero inverse mass. These are unilateral, long-range attachments, which can greatly reduce stretching even at low iteration counts.
 * @param[in] tetherGive Because tether constraints are so effective at reducing stiffness, it can be useful to allow a small amount of extension before the constraint activates.
 * @param[in] pressure If > 0.0f then a volume (pressure) constraint will also be added to the asset, the rest volume and stiffness will be automatically computed by this function
 * @return A pointer to an asset structure holding the particles and constraints
 */
FLEX_API FlexExtAsset* flexExtCreateClothFromMesh(const float* particles, int numParticles, const int* indices, int numTriangles, float stretchStiffness, float bendStiffness, float tetherStiffness, float tetherGive, float pressure);

/**
 * Create a cloth asset consisting of stretch and bend distance constraints given an indexed triangle mesh. This creates an asset with the same
 * structure as flexExtCreateClothFromMesh(), however tether constraints are not supported, and additional information regarding mesh topology
 * will be stored with the asset.
 *
 * @note: Typically each instance of a tearable cloth mesh will have it's own asset. This is because the asset holds the topology of the mesh which is
 * unique for each instance.
 *
 * @param[in] particles Positions and masses of the particles in the format [x, y, z, 1/m]
 * @param[in] numParticles The number of particles
 * @param[in] maxParticles The maximum number of particles for this asset, this will limit the amount of tearing that can be performed.
 * @param[in] indices The triangle indices, these should be 'welded' using flexExtCreateWeldedMeshIndices() first
 * @param[in] numTriangles The number of triangles
 * @param[in] stretchStiffness The stiffness coefficient for stretch constraints
 * @param[in] bendStiffness The stiffness coefficient used for bending constraints
 * @param[in] pressure If > 0.0f then a volume (pressure) constraint will also be added to the asset, the rest volume and stiffness will be automatically computed by this function
 * @return A pointer to an asset structure holding the particles and constraints
 */
FLEX_API FlexExtAsset* flexExtCreateTearingClothFromMesh(const float* particles, int numParticles, int maxParticles, const int* indices, int numTriangles, float stretchStiffness, float bendStiffness, float pressure);

/**
 * Destroy an asset created with flexExtCreateTearingClothFromMesh()
 * @param[in] asset The asset to be destroyed.
 */
FLEX_API void flexExtDestroyTearingCloth(FlexExtAsset* asset);

/**
 * Particles and vertices may need to be copied during tearing. Because the user may maintain particle data 
 * outside of Flex, this structure describes how to update the particle data. The application should copy each 
 * existing particle given by srcIndex (in the asset's particle array) to the destIndex (also in the asset's array).
 */
struct FlexExtTearingParticleClone
{
	int srcIndex;	
	int destIndex;
};

/**
 * The mesh topology may need to be updated to reference new particles during tearing. Because the user may maintain
 * mesh topology outside of Flex, this structure describes the necessary updates that should be performed on the mesh.
 * The triIndex member is the index of the index to be updated, e.g.:
 * a triIndex value of 4 refers to the index 1 vertex (4%3) of the index 1 triangle (4/3). This entry in the indices
 * array should be updated to point to the newParticleIndex.
 */
struct FlexExtTearingMeshEdit
{
	int triIndex;			// index into the triangle indices array to update
	int newParticleIndex;	// new value for the index
};

/**
 * Perform cloth mesh tearing, this function will calculate the strain on each distance constraint and perform splits if it is
 * above a certain strain threshold (i.e.: length/restLength > maxStrain).
 *
 * @param[in] asset The asset describing the cloth constraint network, this must be created with flexExtCreateTearingClothFromMesh()
 * @param[in] maxStrain The maximum allowable strain on each edge
 * @param[in] maxSplits The maximum number of constraint breaks that will be performed, this controls the 'rate' of mesh tearing
 * @param[in] particleCopies Pointer to an array of FlexExtTearingParticleClone structures that describe the particle copies that need to be performed
 * @param[in] numParticleCopies Pointer to an integer that will have the number of copies performed written to it
 * @param[in] maxCopies The maximum number of particle copies that will be performed, multiple particles copies may be performed in response to one split
 * @param[in] triangleEdits Pointer to an array of FlexExtTearingMeshEdit structures that describe the topology updates that need to be performed
 * @param[in] numTriangleEdits Pointer to an integer that will have the number of topology updates written to it
 * @param[in] maxEdits The maximum number of index buffer edits that will be output
 */
FLEX_API void flexExtTearClothMesh(FlexExtAsset* asset, float maxStrain,  int maxSplits, FlexExtTearingParticleClone* particleCopies, int* numParticleCopies, int maxCopies, FlexExtTearingMeshEdit* triangleEdits, int* numTriangleEdits, int maxEdits);

/**
 * Create a shape body asset from a closed triangle mesh. The mesh is first voxelized at a spacing specified by the radius, and particles are placed at occupied voxels.
 *
 * @param[in] vertices Vertices of the triangle mesh
 * @param[in] numVertices The number of vertices
 * @param[in] indices The triangle indices
 * @param[in] numTriangleIndices The number of triangles indices (triangles*3)
 * @param[in] radius The spacing used for voxelization, note that the number of voxels grows proportional to the inverse cube of radius, currently this method limits construction to resolutions < 64^3
 * @param[in] expand Particles will be moved inwards (if negative) or outwards (if positive) from the surface of the mesh according to this factor
 * @return A pointer to an asset structure holding the particles and constraints
 */
FLEX_API FlexExtAsset* flexExtCreateRigidFromMesh(const float* vertices, int numVertices, const int* indices, int numTriangleIndices, float radius, float expand);

/**
* Create a shape body asset from a closed triangle mesh. The mesh is first voxelized at a spacing specified by the radius, and particles are placed at occupied voxels.
*
* @param[in] vertices Vertices of the triangle mesh
* @param[in] numVertices The number of vertices
* @param[in] indices The triangle indices
* @param[in] numTriangleIndices The number of triangles indices (triangles*3)
* @param[in] particleSpacing The spacing to use when creating particles
* @param[in] volumeSampling Control the resolution the mesh is voxelized at in order to generate interior sampling, if the mesh is not closed then this should be set to zero and surface sampling should be used instead
* @param[in] surfaceSampling Controls how many samples are taken of the mesh surface, this is useful to ensure fine features of the mesh are represented by particles, or if the mesh is not closed 
* @param[in] clusterSpacing The spacing for shape-matching clusters, should be at least the particle spacing
* @param[in] clusterRadius Controls the overall size of the clusters, this controls how much overlap  the clusters have which affects how smooth the final deformation is, if parts of the body are detaching then it means the clusters are not overlapping sufficiently to form a fully connected set of clusters
* @param[in] clusterStiffness Controls the stiffness of the resulting clusters
* @param[in] linkRadius Any particles below this distance will have additional distance constraints created between them
* @param[in] linkStiffness The stiffness of distance links
* @return A pointer to an asset structure holding the particles and constraints
*/
FLEX_API FlexExtAsset* flexExtCreateSoftFromMesh(const float* vertices, int numVertices, const int* indices, int numTriangleIndices, float particleSpacing, float volumeSampling, float surfaceSampling, float clusterSpacing, float clusterRadius, float clusterStiffness, float linkRadius, float linkStiffness);

/**
 * Frees all memory associated with an asset created by one of the creation methods
 * param[in] asset The asset to destroy.
 */
FLEX_API void flexExtDestroyAsset(FlexExtAsset* asset);

/**
* Creates information for linear blend skining a graphics mesh to a set of transforms (bones)
*
* @param[in] vertices Vertices of the triangle mesh
* @param[in] numVertices The number of vertices
* @param[in] bones Pointer to an array of vec3 positions representing the bone positions
* @param[in] numBones Then number of bones
* @param[in] falloff The speed at which the bone's influence on a vertex falls off with distance
* @param[in] maxDistance The maximum distance a bone can be from a vertex before it will not influence it any more
* @param[out] skinningWeights The normalized weights for each bone, there are up to 4 weights per-vertex so this should be numVertices*4 in length
* @param[out] skinningIndices The indices of each bone corresponding to the skinning weight, will be -1 if this weight is not used
*/
FLEX_API void flexExtCreateSoftMeshSkinning(const float* vertices, int numVertices, const float* bones, int numBones, float falloff, float maxDistance, float* skinningWeights, int* skinningIndices);

/**
 * Creates a wrapper object around a Flex solver that can hold assets / instances, the container manages sending and retrieving partical data from the solver
 *
 * @param[in] solver The solver to wrap
 * @param[in] maxParticles The maximum number of particles to manage
 * @return A pointer to the new container
 */
FLEX_API FlexExtContainer* flexExtCreateContainer(FlexSolver* solver, int maxParticles);

/**
 * Frees all memory associated with a container
 *
 * @param[in] container The container to destroy
 */
FLEX_API void flexExtDestroyContainer(FlexExtContainer* container);

/**
 * Allocates particles in the container.
 *
 * @param[in] container The container to allocate out of
 * @param[in] n The number of particles to allocate
 * @param[out] indices An n-length array of ints that will store the indices to the allocated particles
 */
FLEX_API int  flexExtAllocParticles(FlexExtContainer* container, int n, int* indices);

/**
 * Free allocated particles
 *
 * @param[in] container The container to free from
 * @param[in] n The number of particles to free
 * @param[in] indices The indices of the particles to free
 */
FLEX_API void flexExtFreeParticles(FlexExtContainer* container, int n, const int* indices);


/**
 * Retrives the indices of all active particles
 *
 * @param[in] container The container to free from
 * @param[out] indices Returns the number of active particles
 * @return The number of active particles
 */
FLEX_API int flexExtGetActiveList(FlexExtContainer* container, int* indices);

/**
 * Creates an instance of an asset, the container will internally store a reference to the asset so it should remain valid for the instance lifetime. This
 * method will allocate particles for the asset, assign their initial positions, velocity and phase.
 *
 * @param[in] container The container to spawn into
 * @param[in] asset The asset to be spawned
 * @param[in] transform A pointer to a 4x4 column major, column vector transform that specifies the initial world space configuration of the particles
 * @param[in] vx The velocity of the particles along the x axis
 * @param[in] vy The velocity of the particles along the y axis
 * @param[in] vz The velocity of the particles along the z axis
 * @param[in] phase The phase used for the particles
 * @param[in] invMassScale A factor applied to the per particle inverse mass
 * @return A pointer to the instance of the asset
 */
FLEX_API FlexExtInstance* flexExtCreateInstance(FlexExtContainer* container, const FlexExtAsset* asset, const float* transform, float vx, float vy, float vz, int phase, float invMassScale);

/** Destoy an instance of an asset
 *
 * @param[in] container The container the instance belongs to
 * @param[in] instance The instance to destroy
 */
FLEX_API void flexExtDestroyInstance(FlexExtContainer* container, const FlexExtInstance* instance);

/** Notifies the container that asset data has changed and needs to be sent to the GPU
 *  this should be called if the constrains for an existing asset are modified by the user
 *
 * @param[in] container The container the instance referencing the asset belongs to
 * @param[in] asset The asset which was modified (can be NULL)
 */
FLEX_API void flexExtNotifyAssetChanged(FlexExtContainer* container, const FlexExtAsset* asset);

/** 
 * Returns pointers to the internal data stored by the container. These are host-memory pointers, and will 
 * remain valid until the container is destroyed. They can be used to read and write particle data, but *only*
 * after suitable synchronization. See flexExtTickContainer() for details.
 *
  @param container The container whose data should be accessed
  @param particles Receives a pointer to the particle position / mass data
  @param restParticles Receives a pointer to the particle's rest position (used for self collision culling)
  @param velocities Receives a pointer to the particle velocity data
  @param phases Receives a pointer to the particle phase data
  @param normals Receives a pointer to the particle normal data with 16 byte stride in format [nx, ny, nz, nw]
 */
FLEX_API void flexExtGetParticleData(FlexExtContainer* container, float** particles, float** restParticles, float** velocities, int** phases, float** normals);

/** 
 * Access triangle constraint data, see flexExtGetParticleData() for notes on ownership.
 *
 * @param container The container to retrive from
 * @param indices Receives a pointer to the array of triangle index data
 * @param normals Receives a pointer to an array of triangle normal data stored with 16 byte stride, i.e.: [nx, ny, nz]
 */
FLEX_API void flexExtGetTriangleData(FlexExtContainer* container, int** indices, float** normals);

/** 
 * Access shape body constraint data, see flexExtGetParticleData() for notes on ownership.
 *
 * @param container The container to retrive from
 * @param rotations Receives a pointer to the array quaternion rotation data in [x, y z, w] format
 * @param positions Receives a pointer to an array of shape body translations in [x, y, z] format
 */
FLEX_API void flexExtGetShapeData(FlexExtContainer* container, float** rotations, float** positions);

/** 
 * Get the container bounds
 *
 * @param container The container to retrive from
 * @param lower Pointer to a 3-vector that will receive the container lower bounds
 * @param upper Pointer to a 3-vector that will receive the container upper bounds 
 */
FLEX_API void flexExtGetBoundsData(FlexExtContainer* container, float* lower, float* upper);

/**
 * Updates the container, applies force fields, steps the solver forward in time, updates the host with the results synchronously.
 * This is a helper function which performs a synchronous update using the following flow.
 *
    \code{.c}
		// async update GPU data
		flexExtPushToDevice(container);

		// update solver
		flexUpdateSolver(container, dt, iterations);

		// async read data back to CPU
		flexExtPullFromDevice(container);

		// insert and wait on fence
		flexSetFence();
		flexWaitFence();

		// read / write particle data on CPU
		flexExtGetParticleData(container, particles, velocities, phases, normals);

  \endcode
  @param[in] container The container to update
  @param[in] dt The time-step in seconds
  @param[in] numSubsteps The number of substeps to perform
  @param[in] timers Pointer to a Flex profiling structure, see flexUpdateSolver()
 */
FLEX_API void flexExtTickContainer(FlexExtContainer* container, float dt, int numSubsteps, FlexTimers* timers=0);

/**
 * Updates the device asynchronously, transfers any particle and constraint changes to the flex solver, 
 * expected to be called in the following sequence: flexExtPushToDevice, flexUpdateSolver, flexExtPullFromDevice, flexSynchronize
 * @param[in] container The container to update
 */
FLEX_API void flexExtPushToDevice(FlexExtContainer* container);

/**
 * Updates the host asynchronously, transfers particle and constraint data back to he host, 
 * expected to be called in the following sequence: flexExtPushToDevice, flexUpdateSolver, flexExtPullFromDevice, flexSynchronize
 * @param[in] container The container to update
 */ 
FLEX_API void flexExtPullFromDevice(FlexExtContainer* container);

/**
 * Synchronizes the per-instance data with the container's data, should be called after the synchronization with the solver read backs are complete
 *
 * @param[in] container The instances belonging to this container will be updated
 */ 
FLEX_API void flexExtUpdateInstances(FlexExtContainer* container);

/*
 * Set force fields on the container
 * @param[in] container The container to update
 * @param[in] forceFields A pointer to an array of force field data, may be host or GPU memory
 * @param[in] numForceFields The number of force fields to send to the device
 * @param[in] source The memory space to copy the force fields from
 */
FLEX_API void flexExtSetForceFields(FlexExtContainer* container, const FlexExtForceField* forceFields, int numForceFields, FlexMemory source);

} // extern "C"

#endif // FLEX_EXT_H

