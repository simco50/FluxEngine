#include "FluxEngine.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "External/Stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "External/Stb/stb_image_write.h"
#include "FileSystem/File/PhysicalFile.h"

#include "DDSLoader.h"

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
	AUTOPROFILE_DESC(Image_Load, inputStream.GetSource().c_str());

	std::string extenstion = Paths::GetFileExtenstion(inputStream.GetSource());
	bool success = false;
	if (extenstion == "dds")
	{
		success = LoadDds(inputStream);
	}
	//If not one of the above, load with Stbi by default (jpg, png, tga, bmp, ...)
	else
	{
		success = LoadStbi(inputStream);
	}
	SetMemoryUsage((unsigned int)m_Pixels.size());
	return success;
}

bool Image::Save(OutputStream& outputStream)
{
	return SavePng(outputStream);
}

bool Image::Save(const std::string& filePath)
{
	std::string extension = Paths::GetFileExtenstion(filePath);
	PhysicalFile file(filePath);
	if (file.OpenWrite() == false)
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
	stbi_io_callbacks callbacks;
	callbacks.read = STBI::ReadCallback;
	callbacks.skip = STBI::SkipCallback;
	callbacks.eof = STBI::EofCallback;
	unsigned char*  pPixels = stbi_load_from_callbacks(&callbacks, &inputStream, &m_Width, &m_Height, &m_ActualComponents, m_Components);
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

	SetMemoryUsage((unsigned int)m_Pixels.size());

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

	SetMemoryUsage((unsigned int)m_Pixels.size());

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

bool Image::LoadStbi(InputStream& inputStream)
{
	m_Components = 4;
	m_Depth = 1;
	stbi_io_callbacks callbacks;
	callbacks.read = STBI::ReadCallback;
	callbacks.skip = STBI::SkipCallback;
	callbacks.eof = STBI::EofCallback;
	unsigned char* pPixels = pPixels = stbi_load_from_callbacks(&callbacks, &inputStream, &m_Width, &m_Height, &m_ActualComponents, m_Components);
	if (pPixels == nullptr)
	{
		return false;
	}
	m_Pixels.resize(m_Width * m_Height * m_Components);
	memcpy(m_Pixels.data(), pPixels, m_Pixels.size());
	stbi_image_free(pPixels);

	return true;
}

bool Image::LoadDds(InputStream& inputStream)
{
	char magic[5];
	magic[4] = '\0';
	inputStream.Read(magic, 4);

	if (strcmp(magic, "DDS ") != 0)
	{
		FLUX_LOG(Warning, "[Image::LoadDds] Invalid DDS file magic: %s", magic);
		return false;
	}

	DDS::FileHeader header;
	inputStream.Read(&header, sizeof(DDS::FileHeader));

	if (header.dwSize == sizeof(DDS::FileHeader) &&
		header.ddpf.dwSize == sizeof(DDS::PixelFormatHeader))
	{
		uint32 fourCC = header.ddpf.dwFourCC;
		bool hasDxgi = fourCC == MAKEFOURCC('D', 'X', '1', '0');
		DDS::DX10FileHeader* pDx10Header = nullptr;

		if (hasDxgi)
		{
			DDS::DX10FileHeader dds10Header = {};
			pDx10Header = &dds10Header;
			inputStream.Read(&dds10Header, sizeof(DDS::DX10FileHeader));

			switch (dds10Header.dxgiFormat)
			{
			case DDS::IMAGE_FORMAT::BC1_UNORM:
			case DDS::IMAGE_FORMAT::BC1_UNORM_SRGB:
				fourCC = MAKEFOURCC('D', 'X', 'T', '1');
				break;
			case DDS::IMAGE_FORMAT::BC2_UNORM:
			case DDS::IMAGE_FORMAT::BC2_UNORM_SRGB:
				fourCC = MAKEFOURCC('D', 'X', 'T', '3');
				break;
			case DDS::IMAGE_FORMAT::BC3_UNORM:
			case DDS::IMAGE_FORMAT::BC3_UNORM_SRGB:
				fourCC = MAKEFOURCC('D', 'X', 'T', '5');
				break;
			case DDS::IMAGE_FORMAT::R8G8B8A8_UNORM:
			case DDS::IMAGE_FORMAT::R8G8B8A8_UNORM_SRGB:
				fourCC = 0;
				break;
			default:
				FLUX_LOG(Warning, "[Image::LoadDds] Invalid DXGI Format '%d'", dds10Header.dxgiFormat);
				return false;
			}
			if (dds10Header.dxgiFormat == DDS::IMAGE_FORMAT::BC1_UNORM_SRGB ||
				dds10Header.dxgiFormat == DDS::IMAGE_FORMAT::BC2_UNORM_SRGB ||
				dds10Header.dxgiFormat == DDS::IMAGE_FORMAT::BC3_UNORM_SRGB ||
				dds10Header.dxgiFormat == DDS::IMAGE_FORMAT::R8G8B8A8_UNORM_SRGB)
			{
				m_sRgb = true;
			}
		}

		switch (fourCC)
		{
		case MAKEFOURCC('D', 'X', 'T', '1'):
			m_CompressionFormat = ImageCompressionFormat::DXT1;
			m_Components = 3;
			break;

		case MAKEFOURCC('D', 'X', 'T', '3'):
			m_CompressionFormat = ImageCompressionFormat::DXT3;
			m_Components = 4;
			break;

		case MAKEFOURCC('D', 'X', 'T', '5'):
			m_CompressionFormat = ImageCompressionFormat::DXT5;
			m_Components = 4;
			break;

		case 0:
			if (header.ddpf.dwRGBBitCount != 32 && header.ddpf.dwRGBBitCount != 24 &&
				header.ddpf.dwRGBBitCount != 16)
			{
				return false;
			}
			m_CompressionFormat = ImageCompressionFormat::RGBA;
			m_Components = 4;
			break;

		default:
			FLUX_LOG(Warning, "[Image::LoadDds] Unrecognized DDS image format");
			return false;
		}

		// Is it a cube map or texture array? If so determine the size of the image chain.
		bool isCubemap = (header.dwCaps2 & DDS::CUBEMAP_ATTRIBUTE::DDSCAPS2_ALL_FACES) != 0 || (hasDxgi && (pDx10Header->miscFlag & 0x4) != 0);
		unsigned imageChainCount = 1;
		if (isCubemap)
			imageChainCount = 6;
		else if (hasDxgi && pDx10Header->arraySize > 1)
		{
			imageChainCount = pDx10Header->arraySize;
			m_IsArray = true;
		}

		unsigned int dataSize = 0;
		if (m_CompressionFormat != ImageCompressionFormat::RGBA)
		{
			const unsigned blockSize = m_CompressionFormat == ImageCompressionFormat::DXT1 ? 8 : 16;

			unsigned blocksWide = (header.dwWidth + 3) / 4;
			unsigned blocksHeight = (header.dwHeight + 3) / 4;
			dataSize = blocksWide * blocksHeight * blockSize;

			// Calculate mip data size
			unsigned x = header.dwWidth / 2;
			unsigned y = header.dwHeight / 2;
			unsigned z = header.dwDepth / 2;
			for (unsigned level = header.dwMipMapCount; level > 1; x /= 2, y /= 2, z /= 2, --level)
			{
				blocksWide = (Math::Max(x, 1U) + 3) / 4;
				blocksHeight = (Math::Max(y, 1U) + 3) / 4;
				dataSize += blockSize * blocksWide * blocksHeight * Math::Max(z, 1U);
			}
		}
		else
		{
			dataSize = (header.ddpf.dwRGBBitCount / 8) * header.dwWidth * header.dwHeight * Math::Max(header.dwDepth, 1U);
			// Calculate mip data size
			unsigned x = header.dwWidth / 2;
			unsigned y = header.dwHeight / 2;
			unsigned z = header.dwDepth / 2;
			for (unsigned level = header.dwMipMapCount; level > 1; x /= 2, y /= 2, z /= 2, --level)
				dataSize += (header.ddpf.dwRGBBitCount / 8) * Math::Max(x, 1U) *  Math::Max(y, 1U) *  Math::Max(z, 1U);
		}

		m_Pixels.resize(dataSize);
		m_Width = header.dwWidth;
		m_Height = header.dwHeight;
		m_Depth = header.dwDepth;
		m_CompressedMipLevels = Math::Max(1, (int)header.dwMipMapCount);
		inputStream.Read(m_Pixels.data(), m_Pixels.size());
	}
	else
	{
		FLUX_LOG(Warning, "[Image::LoadDds] Invalid data structure sizes");
		return false;
	}
	return true;
}

CompressedLevel Image::GetCompressedLevel(const int level) const
{
	CompressedLevel compressedLevel;
	if (m_CompressionFormat == ImageCompressionFormat::NONE)
	{
		FLUX_LOG(Warning, "[Image::GetCompressedLevel] Image is not compressed");
		return CompressedLevel();
	}
	if (level >= m_CompressedMipLevels)
	{
		FLUX_LOG(Warning, "[Image::GetCompressedLevel] Requested level '%d' is greater than the amount of levels '%d'", level, m_CompressedMipLevels);
		return CompressedLevel();
	}
	compressedLevel.Width = m_Width;
	compressedLevel.Height = m_Height;
	compressedLevel.Depth = m_Depth;
	compressedLevel.Format = m_CompressionFormat;
	compressedLevel.DataSize = 0;
	compressedLevel.pData = (char*)m_Pixels.data();
	compressedLevel.CalculateSize();

	for (int i = 0; i < level; ++i)
	{
		compressedLevel.MoveNext();
	}
	return compressedLevel;
}

void CompressedLevel::CalculateSize()
{
	uint32 originalSize = DataSize;

	Width = Math::Max(1, Width);
	Height = Math::Max(1, Height);
	Depth = Math::Max(1, Depth);

	if (Format == ImageCompressionFormat::RGBA)
	{
		BlockSize = 4;
		RowSize = Width * BlockSize;
		Rows = Height;
		DataSize = Depth * Rows * RowSize;
	}
	else if (Format == ImageCompressionFormat::DXT1 ||
		Format == ImageCompressionFormat::DXT3 ||
		Format == ImageCompressionFormat::DXT5 ||
		Format == ImageCompressionFormat::ETC1)
	{
		BlockSize = (Format == ImageCompressionFormat::DXT1 || Format == ImageCompressionFormat::ETC1) ? 8 : 16;
		RowSize = ((Width + 3) / 4) * BlockSize;
		Rows = (Height + 3) / 4;
		DataSize = Depth * Rows * RowSize;
	}
	else
	{
		BlockSize = (Format == ImageCompressionFormat::PVRTC_RGB_4BPP) ? 2 : 4;
		int dataWidth = Math::Max(Width, BlockSize == 2 ? 16 : 8);
		int dataHeight = Math::Max(Height, 8);
		DataSize = (dataWidth * dataHeight * BlockSize + 7) >> 3;
		Rows = dataHeight;
		RowSize = DataSize / Rows;
	}
	pData += originalSize;
}

bool CompressedLevel::MoveNext()
{
	Width /= 2;
	Height /= 2;
	Depth /= 2;

	if (Width == 0 && Height == 0 && Depth == 0)
	{
		return false;
	}

	CalculateSize();
	return true;
}