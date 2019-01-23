#pragma once
#include "Texture.h"

class Texture3D : public Texture
{
	FLUX_OBJECT(Texture3D, Texture)

public:
	explicit Texture3D(Context* pContext);
	virtual ~Texture3D();

	virtual void Release() override;
	virtual bool Load(InputStream& inputStream) override;
	virtual bool Resolve(bool /*force*/) override { return false; }

	bool SetSize(int width, int height, int depth, unsigned int format, TextureUsage usage, int multiSample, void* pTexture);
	bool SetData(unsigned int mipLevel, int x, int y, int z, int width, int height, int depth, const void* pData);
	bool SetImage(const Image& image);

private:
	virtual bool Create() override;
};