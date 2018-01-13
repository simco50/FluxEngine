#pragma once
#include "Scenegraph/Component.h"

class AudioListener : public Component
{
	FLUX_OBJECT(AudioListener, Component)

public:
	AudioListener();
	~AudioListener();
private:
	virtual void OnNodeSet(SceneNode* pNode) override;
	void Update() override; 

	FMOD::System* m_pFmodSystem = nullptr;
	FMOD_VECTOR m_LastPosition;
};

