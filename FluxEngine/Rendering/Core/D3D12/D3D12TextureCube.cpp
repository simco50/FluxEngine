#include "FluxEngine.h"
#include "../TextureCube.h"
#include "../Graphics.h"
#include "D3D12GraphicsImpl.h"
#include "Content/Image.h"
#include "../RenderTarget.h"
#include "../Texture2D.h"

TextureCube::TextureCube(Context* pContext) :
	Texture(pContext)
{

}

TextureCube::~TextureCube()
{
	TextureCube::Release();
}

void TextureCube::Release()
{
	
}

bool TextureCube::Load(InputStream& inputStream)
{
	AUTOPROFILE(TextureCube_Load);
	return false;
}

bool TextureCube::SetSize(int width, int height, unsigned int format, TextureUsage usage, int multiSample, void* pTexture)
{
	return false;
}

bool TextureCube::SetData(CubeMapFace face, unsigned int mipLevel, int x, int y, int width, int height, const void* pData)
{
	return false;
}

bool TextureCube::SetImage(CubeMapFace face, const Image& image)
{
	return false;
}

bool TextureCube::SetImageChain(const Image& image)
{
	return false;
}

bool TextureCube::Create()
{
	return false;
}

bool TextureCube::Resolve(bool force)
{
	return false;
}