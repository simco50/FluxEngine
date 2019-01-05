#pragma once
#include "Texture.h"

class RenderTarget;
class Camera;
class Texture2D;

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

	virtual void Release() override;
	virtual bool Load(InputStream& inputStream) override;
	virtual bool Resolve(bool force) override;

	bool SetSize(int width, int height, unsigned int format, TextureUsage usage, int multiSample, void* pTexture);
	bool SetData(CubeMapFace face, unsigned int mipLevel, int x, int y, int width, int height, const void* pData);
	bool SetImage(CubeMapFace face, const Image& image);
	bool SetImageChain(const Image& image);

	RenderTarget* GetRenderTarget(const CubeMapFace face) const { return m_RenderTargets[(int)face].get(); }

private:
	virtual bool Create() override;

	std::array<std::unique_ptr<RenderTarget>, (int)CubeMapFace::MAX> m_RenderTargets = {};
};