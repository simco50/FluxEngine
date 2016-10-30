#pragma once

class MeshFilter;

namespace FlexHelper
{
	struct FlexMeshInstance
	{
		MeshFilter* pMeshFilter;
		vector<Vector3> RigidRestPoses;
		int Offset;
		int ParticleStart;
		int ParticleCount;

		//Skinning
		vector<int> SkinningIndices;
		vector<float> SkinningWeights;
	};

	inline void FlexMessageCallback(FlexErrorSeverity type, const char* msg, const char* file, int line)
	{
		UNREFERENCED_PARAMETER(file);
		UNREFERENCED_PARAMETER(line);

		wstringstream str;
		str << "[Nvidia Flex] " << msg;
		switch (type)
		{
		case eFlexLogError:
			DebugLog::Log(str.str(), LogType::ERROR);
			break;
		case eFlexLogInfo:
			DebugLog::Log(str.str(), LogType::INFO);
			break;
		case eFlexLogWarning:
			DebugLog::Log(str.str(), LogType::WARNING);
			break;
		case eFlexLogDebug:
			DebugLog::Log(str.str(), LogType::INFO);
			break;
		case eFlexLogAll:
			break;
		default:
			break;
		}
	}
}