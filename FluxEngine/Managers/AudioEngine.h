#pragma once
#include "../Helpers/Singleton.h"

class AudioEngine : public Singleton<AudioEngine>
{
public:
	AudioEngine();
	~AudioEngine();

	FMOD::Sound* LoadSound(const std::string& filePath, const FMOD_MODE mode, FMOD_CREATESOUNDEXINFO* exInfo);
	FMOD::System* GetSystem() const { return m_pSystem; }

	void Update();

private:
	std::map<std::string, FMOD::Sound*> m_Sounds;
	FMOD::System* m_pSystem = nullptr;
};

