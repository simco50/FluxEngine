#include "FluxEngine.h"
#include "HDRImage.h"

#include "FileSystem/File/PhysicalFile.h"

#include <SDL_surface.h>
#include "External/Stb/stb_image.h"

#define TINYEXR_IMPLEMENTATION
#include "External/TinyExr/tinyexr.h"

namespace STB
{
	int ReadCallback(void* pUser, char* pData, int size)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream == nullptr)
		{
			return 0;
		}
		return (int)pStream->Read(pData, (size_t)size);
	}

	void SkipCallback(void* pUser, int n)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream)
		{
			pStream->MovePointer(n);
		}
	}

	int EofCallback(void* pUser)
	{
		InputStream* pStream = (InputStream*)pUser;
		if (pStream == nullptr)
		{
			return 1;
		}
		return pStream->GetPointer() >= pStream->GetSize() ? 1 : 0;
	}
}

HDRImage::HDRImage(Context* pContext)
	: Resource(pContext)
{

}

HDRImage::~HDRImage()
{

}

bool HDRImage::Load(InputStream& inputStream)
{
	AUTOPROFILE_DESC(Image_Load, inputStream.GetSource().c_str());

	stbi_io_callbacks callbacks;
	callbacks.read = STB::ReadCallback;
	callbacks.skip = STB::SkipCallback;
	callbacks.eof = STB::EofCallback;
	if (stbi_is_hdr_from_callbacks(&callbacks, &inputStream))
	{
		return LoadStbi(inputStream);
	}
	else if (Paths::GetFileExtenstion(inputStream.GetSource()) == "exr")
	{
		return LoadExr(inputStream);
	}
	return false;
}

bool HDRImage::SetSize(const int x, const int y, const int components)
{
	m_Width = x;
	m_Height = y;
	m_Components = components;
	m_Pixels.clear();
	m_Pixels.resize(x * y * components);

	SetMemoryUsage((unsigned int)m_Pixels.size());

	return true;
}

bool HDRImage::SetData(const float* pPixels)
{
	memcpy(m_Pixels.data(), pPixels, m_Pixels.size() * m_Components);
	return true;
}

bool HDRImage::SetPixel(const int x, const int y, const Vector3& value)
{
	x; y; value;
	return false;
}

Vector3 HDRImage::GetPixel(const int x, const int y) const
{
	x;
	y;
	Vector3 c;
	return c;
}

const float* HDRImage::GetData() const
{
	return m_Pixels.data();
}

bool HDRImage::LoadStbi(InputStream& inputStream)
{
	m_Components = 3;
	stbi_io_callbacks callbacks;
	callbacks.read = STB::ReadCallback;
	callbacks.skip = STB::SkipCallback;
	callbacks.eof = STB::EofCallback;
	int components = 0;
	float* pPixels = stbi_loadf_from_callbacks(&callbacks, &inputStream, &m_Width, &m_Height, &components, m_Components);
	if (pPixels == nullptr)
	{
		return false;
	}
	m_Pixels.resize(m_Width * m_Height * m_Components);
	memcpy(m_Pixels.data(), pPixels, m_Pixels.size());
	stbi_image_free(pPixels);

	return true;
}

bool HDRImage::LoadExr(InputStream& inputStream)
{

	std::vector<unsigned char> buffer;
	inputStream.ReadAllBytes(buffer);

	const char* errorMessage = nullptr;
	EXRVersion exrVersion;

	int result = ParseEXRVersionFromMemory(&exrVersion, buffer.data(), buffer.size());
	if (result != 0)
	{
		FLUX_LOG(Warning, "[HDRImage::LoadExr] Failed to read Exr version");
		return false;
	}

	if (exrVersion.multipart)
	{
		FLUX_LOG(Warning, "[HDRImage::LoadExr] Exr file is multipart. This is not supported");
		return false;
	}

	EXRHeader exrHeader;
	InitEXRHeader(&exrHeader);

	result = ParseEXRHeaderFromMemory(&exrHeader, &exrVersion, buffer.data(), buffer.size(), &errorMessage);
	if (result != 0)
	{
		FLUX_LOG(Warning, "[HDRImage::LoadExr] Failed to parse Exr header: %s", errorMessage);
		FreeEXRErrorMessage(errorMessage);
		return false;
	}

	//NOTE: Read HALF channel as FLOAT. Exr has 16 bit floats instead of the regular 32 bit floats.
	//Doing this essentially doubles the amount of memory so it's worth looking into getting this to be more flexible
	for (int i = 0; i < exrHeader.num_channels; i++)
	{
		if (exrHeader.pixel_types[i] == TINYEXR_PIXELTYPE_HALF)
		{
			exrHeader.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
		}
	}

	EXRImage exrImage;
	InitEXRImage(&exrImage);

	result = LoadEXRImageFromMemory(&exrImage, &exrHeader, buffer.data(), buffer.size(), &errorMessage);
	if (result != 0)
	{
		FLUX_LOG(Warning, "[HDRImage::LoadExr] Failed to load Exr from memory: %s", errorMessage);
		FreeEXRErrorMessage(errorMessage);
		return false;
	}

	m_Width = exrImage.width;
	m_Height = exrImage.height;
	m_Components = 3;
	m_Pixels.resize(m_Width * m_Height * m_Components);

	for (int i = 0; i < m_Width * m_Height; ++i)
	{
		m_Pixels[i * 3 + 0] = *(float*)&exrImage.images[3][i * sizeof(float)];
		m_Pixels[i * 3 + 1] = *(float*)&exrImage.images[2][i * sizeof(float)];
		m_Pixels[i * 3 + 2] = *(float*)&exrImage.images[1][i * sizeof(float)];
	}

	FreeEXRImage(&exrImage);
	FreeEXRHeader(&exrHeader);

	return true;
}
