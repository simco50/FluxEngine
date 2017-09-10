#pragma once
#include "Core/Components/ComponentBase.h"

class AudioSource : public ComponentBase
{
public:
	AudioSource(const wstring& filePath, const FMOD_MODE& mode);
	AudioSource(FMOD::Sound* pSound);
	~AudioSource();

	void Play();
	void PlayOneShot(FMOD::Sound* pSound);
	void Stop();
	void Pause(const bool paused);

	void SetLoop(const bool loop);

	FMOD::Channel* GetChannel() const { return m_pChannel; }

private:
	void Initialize();
	void Update();

	FMOD_MODE m_Mode;
	wstring m_FilePath;
	FMOD::Sound* m_pSound = nullptr;
	FMOD::Channel* m_pChannel = nullptr;

	FMOD::System* m_pFmodSystem = nullptr;
	FMOD_VECTOR m_LastPosition;
};

