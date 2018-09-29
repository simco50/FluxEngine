#include "FluxEngine.h"
#include "AudioListener.h"
#include "AudioEngine.h"
#include "Scenegraph/SceneNode.h"
#include "SceneGraph/Transform.h"

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

	m_LastPosition = m_pNode->GetTransform()->GetWorldPosition();
}

void AudioListener::OnMarkedDirty(const Transform* transform)
{
	Vector3 velocity = (transform->GetWorldPosition() - m_LastPosition) / GameTimer::DeltaTime();

	Vector3 wPos = transform->GetWorldPosition();
	Vector3 fwd = transform->GetForward();
	Vector3 up = transform->GetUp();

	m_pAudio->GetSystem()->set3DListenerAttributes(
		0,
		reinterpret_cast<const FMOD_VECTOR*>(&wPos),
		reinterpret_cast<const FMOD_VECTOR*>(&velocity),
		reinterpret_cast<const FMOD_VECTOR*>(&fwd),
		reinterpret_cast<const FMOD_VECTOR*>(&up)
	);

	m_LastPosition = transform->GetWorldPosition();
}