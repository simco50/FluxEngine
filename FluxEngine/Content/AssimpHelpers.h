#pragma once

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

class AssimpHelpers
{
public:
	static inline Vector3 ToDXVector3(const aiVector3D& vec)
	{
		return Vector3(vec.x, vec.y, vec.z);
	}

	static inline Quaternion TxDXQuaternion(const aiQuaternion& quat)
	{
		return Quaternion(quat.x, quat.y, quat.z, quat.w);
	}

	static inline Matrix ToDXMatrix(const aiMatrix4x4& mat)
	{
		Matrix m;

		m._11 = mat.a1;
		m._21 = mat.a2;
		m._31 = mat.a3;
		m._41 = mat.a4;

		m._12 = mat.b1;
		m._22 = mat.b2;
		m._32 = mat.b3;
		m._42 = mat.b4;

		m._13 = mat.c1;
		m._23 = mat.c2;
		m._33 = mat.c3;
		m._43 = mat.c4;

		m._14 = mat.d1;
		m._24 = mat.d2;
		m._34 = mat.d3;
		m._44 = mat.d4;

		return m;
	}

	static aiQuaternion GetRotation(const aiNodeAnim* pNode, float time)
	{
		if (pNode->mNumRotationKeys == 0)
		{
			return aiQuaternion();
		}
		for (unsigned int i = 0; i < pNode->mNumRotationKeys; i++)
		{
			if (pNode->mRotationKeys[i].mTime == time)
			{
				return pNode->mRotationKeys[i].mValue;
			}
			if (pNode->mRotationKeys[i].mTime > time)
			{
				float t = Math::InverseLerp((float)pNode->mRotationKeys[i - 1].mTime, (float)pNode->mRotationKeys[i].mTime, time);
				check(t >= 0 && t <= 1);
				aiQuaternion output;
				aiQuaterniont<float>::Interpolate(output, pNode->mRotationKeys[i - 1].mValue, pNode->mRotationKeys[i].mValue, t);
				return output;
			}
		}
		return pNode->mRotationKeys[pNode->mNumRotationKeys - 1].mValue;
	}

	static aiVector3D GetScale(const aiNodeAnim* pNode, float time)
	{
		if (pNode->mNumScalingKeys == 0)
		{
			return aiVector3D();
		}
		for (unsigned int i = 0; i < pNode->mNumScalingKeys; i++)
		{
			if (pNode->mScalingKeys[i].mTime == time)
			{
				return pNode->mScalingKeys[i].mValue;
			}
			if (pNode->mScalingKeys[i].mTime > time)
			{
				float t = Math::InverseLerp((float)pNode->mScalingKeys[i - 1].mTime, (float)pNode->mScalingKeys[i].mTime, time);
				check(t >= 0 && t <= 1);
				return pNode->mScalingKeys[i - 1].mValue + (pNode->mScalingKeys[i].mValue - pNode->mScalingKeys[i - 1].mValue) * t;
			}
		}
		return pNode->mScalingKeys[pNode->mNumScalingKeys - 1].mValue;
	}

	static aiVector3D GetPosition(const aiNodeAnim* pNode, const float time)
	{
		if (pNode->mNumPositionKeys == 0)
		{
			return aiVector3D();
		}
		for (unsigned int i = 0; i < pNode->mNumPositionKeys; i++)
		{
			if (pNode->mPositionKeys[i].mTime == time)
			{
				return pNode->mPositionKeys[i].mValue;
			}
			if (pNode->mPositionKeys[i].mTime > time)
			{
				float t = Math::InverseLerp((float)pNode->mPositionKeys[i - 1].mTime, (float)pNode->mPositionKeys[i].mTime, time);
				check(t >= 0 && t <= 1);
				return pNode->mPositionKeys[i - 1].mValue + (pNode->mPositionKeys[i].mValue - pNode->mPositionKeys[i - 1].mValue) * t;
			}
		}
		return pNode->mPositionKeys[pNode->mNumPositionKeys - 1].mValue;
	}

	static const aiScene* LoadScene(InputStream& inputStream, Assimp::Importer& importer)
	{
		std::vector<unsigned char> buffer;
		inputStream.ReadAllBytes(buffer);
		importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, GraphicsConstants::MAX_BONES_PER_VERTEX);
		AUTOPROFILE(Mesh_ImportAssimp);
		return importer.ReadFileFromMemory(buffer.data(), buffer.size(),
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_LimitBoneWeights
		);
	}
};