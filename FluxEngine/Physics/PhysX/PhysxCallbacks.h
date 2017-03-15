#pragma once
#include "../../Core/stdafx.h"

struct FluxPhysxErrorCallback : PxErrorCallback
{
	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		UNREFERENCED_PARAMETER(message);
		UNREFERENCED_PARAMETER(file);
		UNREFERENCED_PARAMETER(line);

		switch (code)
		{
		case PxErrorCode::eNO_ERROR: break;
		case PxErrorCode::eDEBUG_INFO: break;
		case PxErrorCode::eDEBUG_WARNING: break;
		case PxErrorCode::eINVALID_PARAMETER: break;
		case PxErrorCode::eINVALID_OPERATION: break;
		case PxErrorCode::eOUT_OF_MEMORY: 
		case PxErrorCode::eINTERNAL_ERROR: break;
		case PxErrorCode::eABORT: break;
		case PxErrorCode::ePERF_WARNING: break;
		case PxErrorCode::eMASK_ALL: break;
		default: ;
		}

		cout << message << endl;
	}
};

struct FluxPhysxAllocatorCallback : PxAllocatorCallback
{
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		UNREFERENCED_PARAMETER(typeName);
		UNREFERENCED_PARAMETER(filename);
		UNREFERENCED_PARAMETER(line);
		return _aligned_malloc(size, 16);
	}
	virtual void deallocate(void* ptr)
	{
		_aligned_free(ptr);
	}
};

