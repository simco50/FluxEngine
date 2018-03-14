#pragma once
#include "Texture.h"

class Texture2D : public Texture
{
	FLUX_OBJECT(Texture2D, Texture)

public:
	Texture2D(Context* pContext) :
		Texture(pContext)
	{}
	virtual ~Texture2D()
	{}

	bool Load(InputStream& inputStream);
	bool SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture);
	bool SetData(const void* pData);

private:
	virtual bool Create() override;
};