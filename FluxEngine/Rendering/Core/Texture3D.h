#pragma once
#include "Texture.h"

class Texture3D : public Texture
{
	FLUX_OBJECT(Texture3D, Texture)

public:
	Texture3D(Context* pContext);
	virtual ~Texture3D();

	virtual bool Load(InputStream& inputStream) override;

	bool SetSize(const int width, const int height, const int depth, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture);
	bool SetData(const void* pData);
	bool SetData(Image* pImage);

private:
	virtual bool Create() override;
};