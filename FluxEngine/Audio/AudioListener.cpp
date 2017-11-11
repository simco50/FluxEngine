#include "stdafx.h"
#include "AudioListener.h"
#include "AudioEngine.h"
#include "Scenegraph/SceneNode.h"
#include "Core/Components/Transform.h"

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

	XMFLOAT3 pos = m_pNode->GetTransform()->GetWorldPosition();
	FMOD_VECTOR listenerPosition;
	listenerPosition.x = pos.x;
	listenerPosition.y = pos.y;
	listenerPosition.z = pos.z;
	m_LastPosition = listenerPosition;
}

void AudioListener::Update()
{
	XMFLOAT3 pos = m_pNode->GetTransform()->GetWorldPosition();
	FMOD_VECTOR listenerPosition;
	listenerPosition.x = pos.x;
	listenerPosition.y = pos.y;
	listenerPosition.z = pos.z;

	float dt = GameTimer::DeltaTime();
	FMOD_VECTOR velocity;
	velocity.x = (listenerPosition.x - m_LastPosition.x) / dt;
	velocity.y = (listenerPosition.y - m_LastPosition.y) / dt;
	velocity.z = (listenerPosition.z - m_LastPosition.z) / dt;

	XMFLOAT3 fwd = m_pNode->GetTransform()->GetForward();
	FMOD_VECTOR fwdDir;
	fwdDir.x = fwd.x;
	fwdDir.y = fwd.y;
	fwdDir.z = fwd.z;

	XMFLOAT3 up = m_pNode->GetTransform()->GetUp();
	FMOD_VECTOR upDir;
	upDir.x = up.x;
	upDir.y = up.y;
	upDir.z = up.z;

	m_pFmodSystem->set3DListenerAttributes(0, &listenerPosition, &velocity, &fwdDir, &upDir);

	m_LastPosition = listenerPosition;
}