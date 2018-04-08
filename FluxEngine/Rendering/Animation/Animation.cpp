#include "FluxEngine.h"
#include "Animation.h"

Animation::Animation(Context* pContext, const std::string& name, int numNodes, const float duration, const float ticksPerSecond) :
	Resource(pContext),
	m_Name(name), m_DurationInTicks(duration), m_TickPerSecond(ticksPerSecond)
{
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
