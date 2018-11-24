#pragma once

#include "Resource.h"

class HDRImage : public Resource
{
	FLUX_OBJECT(HDRImage, Resource)
	DELETE_COPY(HDRImage)

public:
	HDRImage(Context* pContext);
	virtual ~HDRImage();

	virtual bool Load(InputStream& inputStream) override;

	bool SetSize(const int x, const int y, const int components);
	bool SetData(const float* pPixels);
	bool SetPixel(const int x, const int y, const Vector3& value);

	Vector3 GetPixel(const int x, const int y) const;

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	int GetComponents() const { return m_Components; }

	float* GetWritableData() { return m_Pixels.data(); }
	const float* GetData() const;

private:
	bool LoadStbi(InputStream& inputStream);
	bool LoadExr(InputStream& inputStream);

	int m_Width = 0;
	int m_Height = 0;
	int m_Components = 0;
	int m_BBP = 0;
	std::vector<float> m_Pixels;
};