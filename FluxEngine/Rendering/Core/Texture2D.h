#pragma once
#include "Texture.h"

enum class ImageFormat;

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

	void* GetRenderTargetView() const { return m_pRenderTargetView; }

private:
	virtual bool Create() override;

	void* m_pRenderTargetView = nullptr;
};