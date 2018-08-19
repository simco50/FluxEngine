#pragma once
#include "Texture.h"

enum class ImageFormat;
class RenderTarget;

class Texture2D : public Texture
{
	FLUX_OBJECT(Texture2D, Texture)

public:
	Texture2D(Context* pContext);
	virtual ~Texture2D();

	virtual bool Load(InputStream& inputStream) override;
	virtual bool Resolve(bool force) override;
	virtual void Release() override;

	bool SetSize(const int width, const int height, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture);
	bool SetData(const unsigned int mipLevel, int x, int y, int width, int height, const void* pData);
	bool SetImage(const Image& image);

	RenderTarget* GetRenderTarget() const { return m_pRenderTarget.get(); }

private:
	virtual bool Create() override;

	std::unique_ptr<RenderTarget> m_pRenderTarget;
};