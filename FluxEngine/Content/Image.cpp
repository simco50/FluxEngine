#include "FluxEngine.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "External/Stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "External/Stb/stb_image_write.h"

/*namespace STBI
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
}*/

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

	unsigned char* pPixels = nullptr;
	std::vector<unsigned char> buffer;
	inputStream.ReadAllBytes(buffer);
	pPixels = stbi_load_from_memory(buffer.data(), (int)buffer.size(), &m_Width, &m_Height, &m_BytesPerPixel, 4);

	/*stbi_io_callbacks callbacks;
	callbacks.read = STBI::ReadCallback;
	callbacks.skip = STBI::SkipCallback;
	callbacks.eof = STBI::EofCallback;
	pPixels = stbi_load_from_callbacks(&callbacks, &inputStream, &m_Width, &m_Height, &m_BytesPerPixel, 4);*/
	if (pPixels == nullptr)
		return false;
	m_Pixels.resize(m_Width * m_Height * 4);
	memcpy(m_Pixels.data(), pPixels, m_Pixels.size());
	stbi_image_free(pPixels);
	return true;
}

bool Image::Save(OutputStream& outputStream)
{
	int result = stbi_write_png_to_func([](void *context, void *data, int size)
	{
		OutputStream* pStream = (OutputStream*)context;
		if (!pStream->Write((char*)data, size))
			return;
	}, &outputStream, m_Width, m_Height, m_Components, m_Pixels.data(), m_Width * m_Components * m_Depth);
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
