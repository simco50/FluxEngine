#include "stdafx.h"
#include "SoundManager.h"

using namespace std;

SoundManager::SoundManager()
{
	FMOD_RESULT result;

	result = FMOD::System_Create(&m_pSystem);
	Console::LogFmodResult(result);

	result = m_pSystem->init(512, FMOD_INIT_NORMAL, nullptr);
	Console::LogFmodResult(result);
}

SoundManager::~SoundManager()
{
	m_pSystem->release();
}

FMOD::Sound* SoundManager::LoadSound(const std::string& filePath, const FMOD_MODE mode, FMOD_CREATESOUNDEXINFO* exInfo)
{
	auto it = m_Sounds.find(filePath);
	if(it != m_Sounds.end())
		return it->second;

	FMOD::Sound* pSound = nullptr;
	FMOD_RESULT result = m_pSystem->createSound(filePath.c_str(), mode, exInfo, &pSound);
	Console::LogFmodResult(result);
	m_Sounds[filePath] = pSound;
	return pSound;
}