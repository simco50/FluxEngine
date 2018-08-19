#pragma once
#include "Core/Subsystem.h"

class AudioEngine : public Subsystem
{
	FLUX_OBJECT(AudioEngine, Subsystem)

public:
	AudioEngine(Context* pContext);
	~AudioEngine();

	FMOD::System* GetSystem() const { return m_pSystem; }

	void Update();

private:
	FMOD::System* m_pSystem = nullptr;
};