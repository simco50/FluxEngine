#include "FluxEngine.h"
#include "AudioListener.h"
#include "AudioEngine.h"
#include "Scenegraph/SceneNode.h"
#include "SceneGraph/Transform.h"

AudioListener::AudioListener()
{
}

AudioListener::~AudioListener()
{
}

void AudioListener::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);

	m_pFmodSystem = AudioEngine::Instance().GetSystem();
	m_LastPosition = m_pNode->GetTransform()->GetWorldPosition();
}

void AudioListener::OnMarkedDirty(const Transform* transform)
{
	Vector3 velocity = (transform->GetWorldPosition() - m_LastPosition) / GameTimer::DeltaTime();

	m_pFmodSystem->set3DListenerAttributes(
		0, 
		reinterpret_cast<const FMOD_VECTOR*>(&transform->GetWorldPosition()),
		reinterpret_cast<const FMOD_VECTOR*>(&velocity),
		reinterpret_cast<const FMOD_VECTOR*>(&transform->GetForward()),
		reinterpret_cast<const FMOD_VECTOR*>(&transform->GetUp())
	);

	m_LastPosition = transform->GetWorldPosition();
}