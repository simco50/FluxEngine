#pragma once
#include "Content\Resource.h"

class Sound : public Resource
{
	FLUX_OBJECT(Sound, Resource)

public:
	Sound(Context* pContext);
	bool Load(InputStream& inputStream) override;

	void SetMode(const uint32 mode);
	uint32 GetMode() const { return m_Mode; }
	FMOD::Sound* GetSound() const { return m_pSound; }

private:
	FMOD::Sound* m_pSound = nullptr;
	uint32 m_Mode = FMOD_2D;
};