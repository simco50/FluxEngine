#pragma once

class PhysxAllocator : public physx::PxAllocatorCallback
{

public:
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override
	{
		UNREFERENCED_PARAMETER(typeName);
		UNREFERENCED_PARAMETER(filename);
		UNREFERENCED_PARAMETER(line);
		return _aligned_malloc(size, 16);
	}

	virtual void deallocate(void* ptr) override
	{
		_aligned_free(ptr);
	}
};