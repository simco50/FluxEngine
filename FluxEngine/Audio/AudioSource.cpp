#include "stdafx.h"
#include "AudioSource.h"
#include "Scenegraph/SceneNode.h"
#include "Core/Components/Transform.h"
#include "AudioEngine.h"

AudioSource::AudioSource(const wstring& filePath, const FMOD_MODE& mode): m_Mode(mode), m_FilePath(filePath)
{

}

AudioSource::AudioSource(FMOD::Sound* pSound): m_Mode(0), m_pSound(pSound)
{
}

AudioSource::~AudioSource()
{
}

void AudioSource::OnSceneSet(Scene* pScene)
{
	Component::OnSceneSet(pScene);

	m_pFmodSystem = AudioEngine::Instance().GetSystem();
	if (m_pSound == nullptr)
		m_pSound = AudioEngine::Instance().LoadSound(string(m_FilePath.begin(), m_FilePath.end()), m_Mode, nullptr);
}

void AudioSource::Update()
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

	m_pChannel->set3DAttributes(&listenerPosition, &velocity);

	m_LastPosition = listenerPosition;
}

void AudioSource::Play()
{
	if (m_pSound == nullptr)
	{
		FLUX_LOG(INFO, "AudioSource::Play() -> Sound is not set");
		return;
	}

	m_pFmodSystem->playSound(m_pSound, nullptr, false, &m_pChannel);
}

void AudioSource::PlayOneShot(FMOD::Sound* pSound)
{
	if (m_pSound == nullptr)
	{
		FLUX_LOG(INFO, "AudioSource::PlayOneShot() -> Sound is nullptr");
		return;
	}

	m_pFmodSystem->playSound(pSound, nullptr, false, nullptr);
}

void AudioSource::Stop()
{
	if (m_pChannel == nullptr)
	{
		FLUX_LOG(WARNING, "AudioSource::Stop() -> Channel is not set");
		return;
	}

	m_pChannel->stop();
}

void AudioSource::Pause(const bool paused)
{
	if (m_pChannel == nullptr)
	{
		FLUX_LOG(WARNING, "AudioSource::Pause() -> Channel is not set");
		return;
	}

	m_pChannel->setPaused(paused);
}

void AudioSource::SetLoop(const bool loop)
{
	if (m_pChannel == nullptr)
	{
		FLUX_LOG(WARNING, "AudioSource::SetLoop() -> Channel is not set");
		return;
	}

	m_pChannel->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
}
