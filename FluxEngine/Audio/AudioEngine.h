#pragma once
#include "Core/Subsystem.h"

#include <fmod.hpp>

class AudioEngine : public Subsystem
{
	FLUX_OBJECT(AudioEngine, Subsystem)

public:
	AudioEngine(Context* pContext);
	~AudioEngine();

	FMOD::System* GetSystem() const { return m_pSystem; }

	void Update();

	static bool ErrorString(FMOD_RESULT result, std::string& message);

private:
	FMOD::System* m_pSystem = nullptr;
};