#pragma once
#include "Core/Components/ComponentBase.h"

class AudioListener : public ComponentBase
{
public:
	AudioListener();
	~AudioListener();
private:
	void Initialize();
	void Update();

	FMOD::System* m_pFmodSystem = nullptr;
	FMOD_VECTOR m_LastPosition;
};

