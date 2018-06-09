#pragma once
#include "Texture.h"

enum class ImageCompressionFormat;

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
	bool SetData(const unsigned int mipLevel, int x, int y, int width, int height, const void* pData);


private:
	static unsigned int TextureFormatFromCompressionFormat(const ImageCompressionFormat& format);
	virtual bool Create() override;
};
