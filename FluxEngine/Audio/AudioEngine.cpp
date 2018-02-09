#include "FluxEngine.h"
#include "AudioEngine.h"

using namespace std;

AudioEngine::AudioEngine():
	Subsystem(nullptr)
{
	AUTOPROFILE(AudioEngine_Initialize);

	FMOD_RESULT result;

	result = FMOD::System_Create(&m_pSystem);
	FLUX_LOG_FMOD(result);

	result = m_pSystem->init(512, FMOD_INIT_NORMAL, nullptr);
	FLUX_LOG_FMOD(result);
}

AudioEngine::~AudioEngine()
{
	m_pSystem->release();
}

FMOD::Sound* AudioEngine::LoadSound(const std::string& filePath, const FMOD_MODE mode, FMOD_CREATESOUNDEXINFO* exInfo)
{
	auto it = m_Sounds.find(filePath);
	if(it != m_Sounds.end())
		return it->second;

	AUTOPROFILE_DESC(AudioEngine_LoadSound, Paths::GetFileName(filePath));

	FMOD::Sound* pSound = nullptr;
	FMOD_RESULT result = m_pSystem->createSound(filePath.c_str(), mode, exInfo, &pSound);
	FLUX_LOG_FMOD(result);
	m_Sounds[filePath] = pSound;
	return pSound;
}

void AudioEngine::Update()
{
	m_pSystem->update();
}
