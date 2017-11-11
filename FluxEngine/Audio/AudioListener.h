#pragma once
#include "Scenegraph/Component.h"

class AudioListener : public Component
{
public:
	AudioListener();
	~AudioListener();
private:
	virtual void OnNodeSet(SceneNode* pNode) override;
	void Update() override; 

	FMOD::System* m_pFmodSystem = nullptr;
	FMOD_VECTOR m_LastPosition;
};

