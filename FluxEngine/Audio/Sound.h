#pragma once
#include "Content\Resource.h"

class Sound : public Resource 
{
	FLUX_OBJECT(Sound, Resource)

public:
	Sound(Context* pContext);
	bool Load(InputStream& inputStream) override;

	FMOD::Sound* GetSound() const { return m_pSound; }

private:
	FMOD::Sound* m_pSound = nullptr;
};