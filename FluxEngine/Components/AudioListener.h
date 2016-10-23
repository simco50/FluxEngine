#pragma once
#include "ComponentBase.h"

class AudioListener : public ComponentBase
{
public:
	AudioListener();
	~AudioListener();
private:
	void Initialize();
	void Update();
	void Render();

	FMOD::System* m_pFmodSystem = nullptr;
	FMOD_VECTOR m_LastPosition;
};

