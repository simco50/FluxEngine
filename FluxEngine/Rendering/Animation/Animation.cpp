#include "FluxEngine.h"
#include "Animation.h"

#include "Content/AssimpHelpers.h"
#include "Rendering/Core/GraphicsDefines.h"
#include "Async/AsyncTaskQueue.h"
#include "Skeleton.h"

Animation::Animation(Context* pContext, const std::string& name, int numNodes, float duration, float ticksPerSecond)
	: Resource(pContext), m_Name(name), m_DurationInTicks(duration), m_TickPerSecond(ticksPerSecond)
{
	m_NameHash = std::hash<std::string>{} (name);
	m_AnimationNodes.resize(numNodes);
}

Animation::Animation(Context* pContext)
	: Resource(pContext), m_DurationInTicks(0), m_TickPerSecond(0)
{

}

Animation::~Animation()
{

}

bool Animation::Load(InputStream& inputStream)
{
	AUTOPROFILE(Mesh_ProcessAssimpAnimations);

	std::vector<unsigned char> buffer;
	inputStream.ReadAllBytes(buffer);

	Assimp::Importer importer;
	const aiScene* pScene;

	{
		importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, GraphicsConstants::MAX_BONES_PER_VERTEX);
		AUTOPROFILE(Mesh_ImportAssimp);
		pScene = importer.ReadFileFromMemory(buffer.data(), buffer.size(),
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_LimitBoneWeights
		);
	}

	if (pScene->HasAnimations())
	{
		const aiAnimation* pAnimation = pScene->mAnimations[0];

		m_Name = pAnimation->mName.C_Str();
		m_NameHash = std::hash<std::string>{} (m_Name);
		m_DurationInTicks = (float)pAnimation->mDuration;
		m_TickPerSecond = (float)pAnimation->mTicksPerSecond;

		m_AnimationNodes.resize(pAnimation->mNumChannels);

		AsyncTaskQueue* pQueue = GetSubsystem<AsyncTaskQueue>();
		for (unsigned int j = 0; j < pAnimation->mNumChannels; j++)
		{
			const aiNodeAnim* pAnimNode = pAnimation->mChannels[j];
			AsyncTaskDelegate taskFunction = AsyncTaskDelegate::CreateLambda([this, pAnimNode, j](AsyncTask*, int)
			{
				AUTOPROFILE(Animation_Load_Channel);

				AnimationNode& animNode = m_AnimationNodes[j];

				animNode.Name = pAnimNode->mNodeName.C_Str();
				size_t fbxSuffix = animNode.Name.find("_$AssimpFbx$");
				if (fbxSuffix != std::string::npos)
				{
					animNode.Name = animNode.Name.substr(0, fbxSuffix);
				}

				animNode.BoneIndex = -1;

				std::vector<float> keyTimes;
				for (unsigned int k = 0; k < pAnimNode->mNumPositionKeys; k++)
				{
					float time = Math::Max<float>(0, (float)pAnimNode->mPositionKeys[k].mTime);
					keyTimes.push_back(time);
				}
				for (unsigned int k = 0; k < pAnimNode->mNumScalingKeys; k++)
				{
					float time = Math::Max<float>(0, (float)pAnimNode->mScalingKeys[k].mTime);
					keyTimes.push_back(time);
				}
				for (unsigned int k = 0; k < pAnimNode->mNumRotationKeys; k++)
				{
					float time = Math::Max<float>(0, (float)pAnimNode->mRotationKeys[k].mTime);
					keyTimes.push_back(time);
				}
				std::sort(keyTimes.begin(), keyTimes.end());
				keyTimes.erase(std::unique(keyTimes.begin(), keyTimes.end()), keyTimes.end());

				for (float time : keyTimes)
				{
					AUTOPROFILE(Animation_Load_Interpolate);

					AnimationKey key;
					key.Rotation = AssimpHelpers::TxDXQuaternion(AssimpHelpers::GetRotation(pAnimNode, time));
					key.Scale = AssimpHelpers::ToDXVector3(AssimpHelpers::GetScale(pAnimNode, time));
					key.Position = AssimpHelpers::ToDXVector3(AssimpHelpers::GetPosition(pAnimNode, time));
					animNode.Keys.push_back(AnimationNode::KeyPair(time, key));
				}
			});

			pQueue->AddWorkItem(taskFunction);
		}
		pQueue->JoinAll();

		unsigned int memoryUsage = 0;
		for (const auto& node : m_AnimationNodes)
		{
			memoryUsage += (unsigned int)node.Keys.size() * sizeof(AnimationKey);
		}
		SetMemoryUsage(memoryUsage);

		return true;
	}
	else
	{
		return false;
	}
}

void Animation::ResolveBoneIndices(const Skeleton& skeleton)
{
	checkf(skeleton.BoneCount() == m_AnimationNodes.size(), "[Animation::ResolveBoneIndices] Bone count does not match the amount of animation nodes")
	const std::vector<Bone>& bones = skeleton.GetBones();
	for (size_t i = 0; i < m_AnimationNodes.size(); ++i)
	{
		bool found = false;
		for (size_t j = 0; j < bones.size(); ++j)
		{
			if (bones[j].Name == m_AnimationNodes[i].Name)
			{
				m_AnimationNodes[i].BoneIndex = bones[j].Index;
				std::swap(m_AnimationNodes[i], m_AnimationNodes[j]);
				found = true;
				break;
			}
		}
		checkf(found, Printf("[Animation::ResolveBoneIndices] Bone with name '%s' does not exist in Skeleton", m_AnimationNodes[i].Name.c_str()).c_str());
	}
}

void Animation::SetNode(const AnimationNode& node)
{
	m_AnimationNodes[node.BoneIndex] = node;
}

AnimationNode& Animation::GetNode(int boneIndex)
{
	assert(boneIndex < (int)m_AnimationNodes.size());
	return m_AnimationNodes[boneIndex];
}
