#pragma once

#include "Resource.h"

class Image : public Resource
{
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
	unsigned char* GetData() { return m_Pixels.data(); }
	
	SDL_Surface* GetSDLSurface();

private:
	int m_Width = 0;
	int m_Height = 0;
	int m_Components = 0;
	int m_ActualComponents = 0;
	int m_Depth = 1;
	std::vector<unsigned char> m_Pixels;
};