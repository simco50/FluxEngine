#pragma once

class PhysxAllocator : public physx::PxAllocatorCallback
{

public:
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override
	{
#ifdef PHYSX_DEBUG_ALLOCATIONS
		FLUX_LOG(Log, "[PhysxAllocator::allocate()] Allocated %d bytes for %s at %s (%d)", size, typename, filename, line);
#else
		UNREFERENCED_PARAMETER(typeName);
		UNREFERENCED_PARAMETER(filename);
		UNREFERENCED_PARAMETER(line);
#endif

		return _aligned_malloc(size, 16);
	}

	virtual void deallocate(void* ptr) override
	{
		_aligned_free(ptr);
	}
};