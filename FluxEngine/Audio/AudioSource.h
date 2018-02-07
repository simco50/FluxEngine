#pragma once
#include "Scenegraph/Component.h"

class AudioSource : public Component
{
	FLUX_OBJECT(AudioSource, Component)

public:
	AudioSource(const string& filePath, const FMOD_MODE& mode);
	AudioSource(FMOD::Sound* pSound);
	~AudioSource();

	void Play();
	void PlayOneShot(FMOD::Sound* pSound);
	void Stop();
	void Pause(const bool paused);
	void SetLoop(const bool loop);

	FMOD::Channel* GetChannel() const { return m_pChannel; }

private:
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnMarkedDirty(const Transform* transform) override;

	FMOD_MODE m_Mode;
	string m_FilePath;
	FMOD::Sound* m_pSound = nullptr;
	FMOD::Channel* m_pChannel = nullptr;

	FMOD::System* m_pFmodSystem = nullptr;
	Vector3 m_LastPosition;
};