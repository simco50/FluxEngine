#pragma once

#include "Resource.h"

enum class ImageCompressionFormat
{
	NONE = 0,
	RGBA,
	DXT1,
	DXT3,
	DXT5,
	ETC1,
	PVRTC_RGB_2BPP,
	PVRTC_RGBA_2BPP,
	PVRTC_RGB_4BPP,
	PVRTC_RGBA_4BPP,
};

class Image : public Resource
{
	FLUX_OBJECT(Image, Resource)

public:
	Image(Context* pContext);
	virtual ~Image();

	virtual bool Load(InputStream& inputStream) override;
	virtual bool Save(OutputStream& outputStream) override;

	bool LoadLUT(InputStream& inputStream);

	bool Save(const std::string& filePath);
	bool SavePng(OutputStream& outputStream);
	bool SaveBmp(OutputStream& outputStream);
	bool SaveJpg(OutputStream& outputStream, const int quality = 100);
	bool SaveTga(OutputStream& outputStream);

	bool SetSize(const int x, const int y, const int components);
	bool SetData(const unsigned int* pPixels);
	bool SetPixel(const int x, const int y, const Color& color);
	bool SetPixelInt(const int x, const int y, const unsigned int color);

	//void ConvertToRGBA();

	Color GetPixel(const int x, const int y) const;
	unsigned int GetPixelInt(const int x, const int y) const;

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	int GetDepth() const { return m_Depth; }
	int GetComponents() const { return m_Components; }
	int GetActualComponents() const { return m_ActualComponents; }
	int GetMipLevels() const { return m_MipLevels; }
	unsigned char* GetData() { return m_Pixels.data(); }
	bool IsCompressed() const { return m_CompressionFormat != ImageCompressionFormat::NONE; }
	bool IsSRGB() const { return m_sRgb; }
	ImageCompressionFormat GetCompressionFormat() const { return m_CompressionFormat; }
	SDL_Surface* GetSDLSurface();

private:
	bool LoadDds(InputStream& inputStream);
	bool LoadStbi(InputStream& inputStream);

	int m_Width = 0;
	int m_Height = 0;
	int m_Components = 0;
	int m_ActualComponents = 0;
	int m_Depth = 1;
	int m_MipLevels = 1;
	bool m_sRgb = false;
	bool m_IsArray = false;
	ImageCompressionFormat m_CompressionFormat = ImageCompressionFormat::NONE;
	std::vector<unsigned char> m_Pixels;
};