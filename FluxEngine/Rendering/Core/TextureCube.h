#pragma once
#include "Texture.h"

enum class CubeMapFace
{
	POSITIVE_X = 0,
	NEGATIVE_X,
	POSITIVE_Y,
	NEGATIVE_Y,
	POSITIVE_Z,
	NEGATIVE_Z,
	MAX
};

class TextureCube : public Texture
{
	FLUX_OBJECT(TextureCube, Texture)

public:
	TextureCube(Context* pContext);
	virtual ~TextureCube(); 

	virtual bool Load(InputStream& inputStream) override;

	bool SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture);
	bool SetData(const CubeMapFace face, const unsigned int mipLevel, int x, int y, int width, int height, const void* pData);
	bool SetImage(const CubeMapFace face, const Image& image);
	bool SetImageChain(const Image& image);

private:
	virtual bool Create() override;
};