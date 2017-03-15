#include "../../Core/stdafx.h"
#pragma once

struct SimulationFilter
{
	static PxFilterFlags SimulationFilterShader(
		PxFilterObjectAttributes attribute0, PxFilterData filterData0,
		PxFilterObjectAttributes attribute1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		UNREFERENCED_PARAMETER(constantBlock);
		UNREFERENCED_PARAMETER(constantBlockSize);

		// let triggers through
		if (PxFilterObjectIsTrigger(attribute0) || PxFilterObjectIsTrigger(attribute1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}
		// generate contacts for all that were not filtered above
		pairFlags = PxPairFlag::eCONTACT_DEFAULT;

		// trigger the contact callback for pairs (A,B) where
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

		return PxFilterFlag::eDEFAULT;
	}
};