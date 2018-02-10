#pragma once

class PhysxErrorCallback : public physx::PxErrorCallback
{
public:
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
	{
		switch (code)
		{
		case physx::PxErrorCode::eDEBUG_INFO:
			FLUX_LOG(Info, "[PhysX] > %s", message);
			break;
		case physx::PxErrorCode::ePERF_WARNING:
		case physx::PxErrorCode::eDEBUG_WARNING:
			FLUX_LOG(Warning, "[PhysX] > %s", message);
			break;
		case physx::PxErrorCode::eINVALID_PARAMETER:
		case physx::PxErrorCode::eINVALID_OPERATION:
		case physx::PxErrorCode::eOUT_OF_MEMORY:
		case physx::PxErrorCode::eINTERNAL_ERROR:
		case physx::PxErrorCode::eABORT:
			FLUX_LOG(Error, "[PhysX] > %s in '%s' - Line %i", message, file, line);
		case physx::PxErrorCode::eMASK_ALL:
		case physx::PxErrorCode::eNO_ERROR:
		default:
			break;
		}
	}
};