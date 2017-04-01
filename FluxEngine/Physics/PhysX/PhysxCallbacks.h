#pragma once

struct FluxPhysxErrorCallback : PxErrorCallback
{
	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		stringstream str;
		str << "[" << file << "] (" << line << ") " << message;
			
		switch (code)
		{
		case PxErrorCode::eNO_ERROR:
		case PxErrorCode::eDEBUG_INFO:
			Console::Log(str.str());
		case PxErrorCode::eINVALID_PARAMETER:
		case PxErrorCode::eINVALID_OPERATION:
		case PxErrorCode::ePERF_WARNING:
		case PxErrorCode::eDEBUG_WARNING: 
			Console::LogWarning(str.str());
			break;
		case PxErrorCode::eOUT_OF_MEMORY:  
		case PxErrorCode::eINTERNAL_ERROR:
		case PxErrorCode::eABORT:
			Console::LogError(str.str());
		default: ;
		}
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

