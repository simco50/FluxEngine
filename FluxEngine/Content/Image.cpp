#include "FluxEngine.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "External/Stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "External/Stb/stb_image_write.h"
#include "FileSystem/File/PhysicalFile.h"

namespace STBI
{
	int ReadCallback(void* pUser, char* pData, int size)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream == nullptr)
			return 0;
		return (int)pStream->Read(pData, (size_t)size);
	}

	void SkipCallback(void* pUser, int n)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream)
			pStream->MovePointer(n);
	}

	int EofCallback(void* pUser)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream == nullptr)
			return 1;
		return pStream->GetPointer() >= pStream->GetSize() ? 1 : 0;
	}
}

Image::Image(Context* pContext) :
	Resource(pContext)
{

}

Image::~Image()
{

}

bool Image::Load(InputStream& inputStream)
{
	AUTOPROFILE(Image_Load);
	m_Components = 4;
	m_Depth = 1;
	unsigned char* pPixels = nullptr;
	stbi_io_callbacks callbacks;
	callbacks.read = STBI::ReadCallback;
	callbacks.skip = STBI::SkipCallback;
	callbacks.eof = STBI::EofCallback;
	pPixels = stbi_load_from_callbacks(&callbacks, &inputStream, &m_Width, &m_Height, &m_ActualComponents, m_Components);
	if (pPixels == nullptr)
		return false;
	m_Pixels.resize(m_Width * m_Height * m_Components);
	memcpy(m_Pixels.data(), pPixels, m_Pixels.size());
	stbi_image_free(pPixels);
	return true;
}

bool Image::Save(OutputStream& outputStream)
{
	return SavePng(outputStream);
}

bool Image::Save(const std::string& filePath)
{
	std::string extension = Paths::GetFileExtenstion(filePath);
	PhysicalFile file(filePath);
	if (file.Open(FileMode::Write) == false)
		return false;
	if (extension == "png")
		return SavePng(file);
	if(extension == "jpg")
		return SaveJpg(file);
	if (extension == "tga")
		return SaveTga(file);
	if (extension == "bmp")
		return SaveBmp(file);
	FLUX_LOG(Warning, "[Image::Save] > File extension '%s' is not supported", extension.c_str());
	return false;
}

bool Image::LoadLUT(InputStream& inputStream)
{
	AUTOPROFILE(Image_Load);
	m_Components = 4;
	unsigned char* pPixels = nullptr;
	stbi_io_callbacks callbacks;
	callbacks.read = STBI::ReadCallback;
	callbacks.skip = STBI::SkipCallback;
	callbacks.eof = STBI::EofCallback;
	pPixels = stbi_load_from_callbacks(&callbacks, &inputStream, &m_Width, &m_Height, &m_ActualComponents, m_Components);
	if (pPixels == nullptr)
		return false;

	m_Pixels.resize(m_Height * m_Width * m_Components);
	m_Width = m_Depth = m_Height = 16;

	int* c3D = (int*)m_Pixels.data();
	int* c2D = (int*)pPixels;
	int dim = m_Height;
	for (int z = 0; z < dim; ++z)
		for (int y = 0; y < dim; ++y)
			for (int x = 0; x < dim; ++x)
				c3D[x + y * dim + z * dim * dim]
				= c2D[x + y * dim * dim + z * dim];


	stbi_image_free(pPixels);
	return true;
}

bool Image::SavePng(OutputStream& outputStream)
{
	int result = stbi_write_png_to_func([](void *context, void *data, int size)
	{
		OutputStream* pStream = (OutputStream*)context;
		if (!pStream->Write((char*)data, size))
			return;
	}, &outputStream, m_Width, m_Height, m_Components, m_Pixels.data(), m_Width * m_Components * m_Depth);
	return result > 0;
}


bool Image::SaveBmp(OutputStream& outputStream)
{
	int result = stbi_write_bmp_to_func([](void *context, void *data, int size)
	{
		OutputStream* pStream = (OutputStream*)context;
		if (!pStream->Write((char*)data, size))
			return;
	}, &outputStream, m_Width, m_Height, m_Components, m_Pixels.data());
	return result > 0;
}

bool Image::SaveJpg(OutputStream& outputStream, const int quality /*= 100*/)
{
	int result = stbi_write_jpg_to_func([](void *context, void *data, int size)
	{
		OutputStream* pStream = (OutputStream*)context;
		if (!pStream->Write((char*)data, size))
			return;
	}, &outputStream, m_Width, m_Height, m_Components, m_Pixels.data(), quality);
	return result > 0;
}

bool Image::SaveTga(OutputStream& outputStream)
{
	const int quality = 8;
	int result = stbi_write_tga_to_func([](void *context, void *data, int size)
	{
		OutputStream* pStream = (OutputStream*)context;
		if (!pStream->Write((char*)data, size))
			return;
	}, &outputStream, m_Width, m_Height, m_Components, m_Pixels.data());
	return result > 0;
}

bool Image::SetSize(const int x, const int y, const int components)
{
	m_Width = x;
	m_Height = y;
	m_Depth = 1;
	m_Components = components;
	m_Pixels.clear();
	m_Pixels.resize(x * y * components);
	return true;
}

bool Image::SetData(const unsigned int* pPixels)
{
	memcpy(m_Pixels.data(), pPixels, m_Pixels.size() * m_Depth * m_Components);
	return true;
}

bool Image::SetPixel(const int x, const int y, const Color& color)
{
	if (x + y * m_Width >= (int)m_Pixels.size())
		return false;
	unsigned char* pPixel = &m_Pixels[(x + (y * m_Width)) * m_Components * m_Depth];
	for (int i = 0; i < m_Components; ++i)
		pPixel[i] = (unsigned char)(color[i] * 255);
	return true;
}

bool Image::SetPixelInt(const int x, const int y, const unsigned int color)
{
	if (x + y * m_Width >= (int)m_Pixels.size())
		return false;
	unsigned char* pPixel = &m_Pixels[(x + (y * m_Width)) * m_Components * m_Depth];
	for (int i = 0; i < m_Components; ++i)
		pPixel[i] = reinterpret_cast<const unsigned char*>(&color)[i];
	return true;
}

Color Image::GetPixel(const int x, const int y) const
{
	Color c = {};
	if (x + y * m_Width >= (int)m_Pixels.size())
		return c;
	const unsigned char* pPixel = &m_Pixels[(x + (y * m_Width)) * m_Components * m_Depth];
	for (int i = 0; i < m_Components; ++i)
		reinterpret_cast<float*>(&c)[i] = (float)pPixel[i] / 255.0f;
	return c;
}

unsigned int Image::GetPixelInt(const int x, const int y) const
{
	unsigned int c = 0;
	if (x + y * m_Width >= (int)m_Pixels.size())
		return c;
	const unsigned char* pPixel = &m_Pixels[(x + (y * m_Width)) * m_Components * m_Depth];
	for (int i = 0; i < m_Components; ++i)
	{
		c <<= 8;
		c |= pPixel[i];
	}
	c <<= 8 * (4 - m_Components);
	return c;
}

SDL_Surface* Image::GetSDLSurface()
{
	if (m_Pixels.size() == 0)
		return nullptr;

	// Assume little-endian for all the supported platforms
	unsigned rMask = 0x000000ff;
	unsigned gMask = 0x0000ff00;
	unsigned bMask = 0x00ff0000;
	unsigned aMask = 0xff000000;

	SDL_Surface* surface = SDL_CreateRGBSurface(0, m_Width, m_Height, 4 * 8, rMask, gMask, bMask, aMask);
	SDL_LockSurface(surface);

	unsigned char* destination = reinterpret_cast<unsigned char*>(surface->pixels);
	unsigned char* source = m_Pixels.data() ;
	for (int i = 0; i < m_Height; ++i)
	{
		memcpy(destination, source, 4 * m_Width);
		destination += surface->pitch;
		source += 4 * m_Width;
	}

	SDL_UnlockSurface(surface);

	return surface;
}
