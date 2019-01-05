#include "FluxEngine.h"
#include "Animation.h"

#include "Content/AssimpHelpers.h"
#include "Async/AsyncTaskQueue.h"
#include "Skeleton.h"

Animation::Animation(Context* pContext)
	: Resource(pContext), m_DurationInTicks(0), m_TickPerSecond(0)
{

}

Animation::~Animation()
{

}

bool Animation::Load(InputStream& inputStream)
{
	AUTOPROFILE(Animation_Load);

	const aiScene* pScene = nullptr;
	Assimp::Importer importer;
	{
		AUTOPROFILE(Animation_ImportAssimp);
		pScene = AssimpHelpers::LoadScene(inputStream, importer);
	}

	if (pScene->HasAnimations())
	{
		const aiAnimation* pAnimation = pScene->mAnimations[0];

		if (pAnimation->mName.length != 0)
		{
			m_Name = pAnimation->mName.C_Str();
		}
		else
		{
			m_Name = Paths::GetFileNameWithoutExtension(inputStream.GetSource());
		}

		m_NameHash = StringHash(m_Name);
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

				std::vector<float> keyTimes;
				for (unsigned int k = 0; k < pAnimNode->mNumPositionKeys; k++)
				{
					keyTimes.push_back((float)pAnimNode->mPositionKeys[k].mTime);
				}
				for (unsigned int k = 0; k < pAnimNode->mNumScalingKeys; k++)
				{
					keyTimes.push_back((float)pAnimNode->mScalingKeys[k].mTime);
				}
				for (unsigned int k = 0; k < pAnimNode->mNumRotationKeys; k++)
				{
					keyTimes.push_back((float)pAnimNode->mRotationKeys[k].mTime);
				}
				std::sort(keyTimes.begin(), keyTimes.end());
				keyTimes.erase(std::unique(keyTimes.begin(), keyTimes.end()), keyTimes.end());

				for (float time : keyTimes)
				{
					AUTOPROFILE(Animation_Load_Interpolate);

					checkf(time >= 0, "[Animation::Load] Key time is negative");

					AnimationKey key;
					key.Rotation = AssimpHelpers::TxDXQuaternion(AssimpHelpers::GetRotation(pAnimNode, time));
					key.Scale = AssimpHelpers::ToDXVector3(AssimpHelpers::GetScale(pAnimNode, time));
					key.Position = AssimpHelpers::ToDXVector3(AssimpHelpers::GetPosition(pAnimNode, time));

					//Normalize the time
					time /= keyTimes.back();

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
	checkf(skeleton.BoneCount() == m_AnimationNodes.size(), "[Animation::ResolveBoneIndices] Bone count does not match the amount of animation nodes");
	const std::vector<Bone>& bones = skeleton.GetBones();
	for (size_t i = 0; i < m_AnimationNodes.size(); ++i)
	{
		bool found = false;
		for (size_t j = 0; j < bones.size(); ++j)
		{
			if (bones[j].Name == m_AnimationNodes[i].Name)
			{
				m_AnimationNodes[i].BoneIndex = (int)j;
				found = true;
				break;
			}
		}
		checkf(found, Printf("[Animation::ResolveBoneIndices] Bone with name '%s' does not exist in Skeleton", m_AnimationNodes[i].Name.c_str()).c_str());
	}
}