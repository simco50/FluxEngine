#pragma once

#include "Resource.h"

class Image : public Resource
{
public:
	Image(Context* pContext);
	virtual ~Image();

	virtual bool Load(InputStream& inputStream) override;

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	int GetBPP() const { return m_BytesPerPixel; }
	const unsigned char* GetData() const { return m_Pixels.data(); }
	
	SDL_Surface* GetSDLSurface();

private:
	int m_Width = 0;
	int m_Height = 0;
	int m_BytesPerPixel = 0;
	std::vector<unsigned char> m_Pixels;
};