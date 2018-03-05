#include "FluxEngine.h"
#include "AudioSource.h"
#include "Scenegraph/SceneNode.h"
#include "SceneGraph/Transform.h"
#include "AudioEngine.h"

AudioSource::AudioSource(Context* pContext, const std::string& filePath, const FMOD_MODE& mode):
	Component(pContext), m_Mode(mode), m_FilePath(filePath)
{
	AudioEngine* pAudioEngine = pContext->GetSubsystem<AudioEngine>();
	if (m_pSound == nullptr)
		m_pSound = pAudioEngine->LoadSound(m_FilePath, m_Mode, nullptr);
	m_pFmodSystem = pAudioEngine->GetSystem();
}

AudioSource::AudioSource(Context* pContext, FMOD::Sound* pSound): 
	Component(pContext), m_Mode(0), m_pSound(pSound)
{
}

AudioSource::~AudioSource()
{
}

void AudioSource::Play()
{
	if (m_pSound == nullptr)
	{
		FLUX_LOG(Info, "AudioSource::Play() -> Sound is not set");
		return;
	}

	m_pFmodSystem->playSound(m_pSound, nullptr, false, &m_pChannel);
}

void AudioSource::PlayOneShot(FMOD::Sound* pSound)
{
	if (m_pSound == nullptr)
	{
		FLUX_LOG(Info, "AudioSource::PlayOneShot() -> Sound is nullptr");
		return;
	}

	m_pFmodSystem->playSound(pSound, nullptr, false, nullptr);
}

void AudioSource::Stop()
{
	if (m_pChannel == nullptr)
	{
		FLUX_LOG(Warning, "AudioSource::Stop() -> Channel is not set");
		return;
	}

	m_pChannel->stop();
}

void AudioSource::Pause(const bool paused)
{
	if (m_pChannel == nullptr)
	{
		FLUX_LOG(Warning, "AudioSource::Pause() -> Channel is not set");
		return;
	}

	m_pChannel->setPaused(paused);
}

void AudioSource::SetLoop(const bool loop)
{
	if (m_pChannel == nullptr)
	{
		FLUX_LOG(Warning, "AudioSource::SetLoop() -> Channel is not set");
		return;
	}

	m_pChannel->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
}

void AudioSource::OnNodeSet(SceneNode* pNode)
{
	Component::OnNodeSet(pNode);
	m_LastPosition = pNode->GetTransform()->GetWorldPosition();
}

void AudioSource::OnMarkedDirty(const Transform* transform)
{
	Vector3 velocity = (transform->GetWorldPosition() - m_LastPosition) / GameTimer::DeltaTime();

	m_pChannel->set3DAttributes(
		reinterpret_cast<const FMOD_VECTOR*>(&m_pNode->GetTransform()->GetWorldPosition()),
		reinterpret_cast<const FMOD_VECTOR*>(&velocity)
	);

	m_LastPosition = m_pNode->GetTransform()->GetWorldPosition();
}
