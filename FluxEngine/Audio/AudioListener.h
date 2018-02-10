#pragma once
#include "Scenegraph/Component.h"

class AudioListener : public Component
{
	FLUX_OBJECT(AudioListener, Component)

public:
	AudioListener(Context* pContext);
	~AudioListener();

private:
	virtual void OnNodeSet(SceneNode* pNode) override;
	virtual void OnMarkedDirty(const Transform* transform) override;

	FMOD::System* m_pFmodSystem = nullptr;
	Vector3 m_LastPosition;
};

