#include "FluxEngine.h"
#include "AudioListener.h"
#include "AudioEngine.h"
#include "Scenegraph/SceneNode.h"

AudioListener::AudioListener(Context* pContext)
	: Component(pContext)
{
	m_pAudio = pContext->GetSubsystem<AudioEngine>();
}

AudioListener::~AudioListener()
{
}

void AudioListener::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);

	m_LastPosition = m_pNode->GetWorldPosition();
}

void AudioListener::OnMarkedDirty(const SceneNode* pNode)
{
	Vector3 velocity = (pNode->GetWorldPosition() - m_LastPosition);
	if(GameTimer::DeltaTime() > 0)
		velocity /= GameTimer::DeltaTime();

	Vector3 wPos = pNode->GetWorldPosition();
	Vector3 fwd = pNode->GetForward();
	Vector3 up = pNode->GetUp();

	m_pAudio->GetSystem()->set3DListenerAttributes(
		0,
		reinterpret_cast<const FMOD_VECTOR*>(&wPos),
		reinterpret_cast<const FMOD_VECTOR*>(&velocity),
		reinterpret_cast<const FMOD_VECTOR*>(&fwd),
		reinterpret_cast<const FMOD_VECTOR*>(&up)
	);

	m_LastPosition = pNode->GetWorldPosition();
}