#include "FluxEngine.h"
#include "../Texture3D.h"
#include "../Graphics.h"

#include "D3D12GraphicsImpl.h"
#include "Content/Image.h"

Texture3D::Texture3D(Context* pContext)
	: Texture(pContext)
{

}

Texture3D::~Texture3D()
{
	Texture3D::Release();
}

void Texture3D::Release()
{
}

bool Texture3D::Load(InputStream& inputStream)
{
	AUTOPROFILE(Texture3D_Load);
	return false;
}

bool Texture3D::SetSize(int width, int height, int depth, unsigned int format, TextureUsage usage, int multiSample, void* pTexture)
{
	return false;
}

bool Texture3D::SetData(unsigned int mipLevel, int x, int y, int z, int width, int height, int depth, const void* pData)
{
	return false;
}

bool Texture3D::SetImage(const Image& image)
{
	return false;
}

bool Texture3D::Create()
{
	return false;
}
