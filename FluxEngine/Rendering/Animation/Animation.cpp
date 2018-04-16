#include "FluxEngine.h"
#include "Animation.h"

Animation::Animation(Context* pContext, const std::string& name, int numNodes, const float duration, const float ticksPerSecond) :
	Resource(pContext),
	m_Name(name), m_DurationInTicks(duration), m_TickPerSecond(ticksPerSecond)
{
	m_NameHash = std::hash<std::string>{} (name);
	m_AnimationNodes.resize(numNodes);
}

Animation::~Animation()
{

}

bool Animation::Load(InputStream& inputStream)
{
	UNREFERENCED_PARAMETER(inputStream);
	return false;
}

void Animation::SetNode(const AnimationNode& node)
{
	m_AnimationNodes[node.BoneIndex] = node;
}

AnimationNode& Animation::GetNode(const int boneIndex)
{
	assert(boneIndex < (int)m_AnimationNodes.size());
	return m_AnimationNodes[boneIndex];
}
