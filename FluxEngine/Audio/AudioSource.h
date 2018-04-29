#pragma once
#include "Scenegraph/Component.h"

class AudioEngine;
class Sound;

class AudioSource : public Component
{
	FLUX_OBJECT(AudioSource, Component)

public:
	AudioSource(Context* pContext, const std::string& filePath, const FMOD_MODE& mode);
	AudioSource(Context* pContext, Sound* pSound);
	~AudioSource();

	void Play();
	void PlayOneShot(Sound* pSound);
	void Stop();
	void Pause(const bool paused);
	void SetLoop(const bool loop);

	FMOD::Channel* GetChannel() const { return m_pChannel; }
	Sound* GetSound() const { return m_pSound; }

private:
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnMarkedDirty(const Transform* transform) override;

	FMOD_MODE m_Mode;
	std::string m_FilePath;
	Sound* m_pSound = nullptr;
	FMOD::Channel* m_pChannel = nullptr;

	FMOD::System* m_pFmodSystem = nullptr;
	Vector3 m_LastPosition;
};