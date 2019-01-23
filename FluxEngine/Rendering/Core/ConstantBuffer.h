#pragma once
#include "GraphicsObject.h"

class Graphics;

/*
	Register 0 is reserved for per-frame data
	Register 1 is reserved for per-view data
	Register 2 is reserved for per-object data
	Registers 3-8 is free to use

	See 'Rendering/Core/GraphicsDefines.h'
*/

class ConstantBuffer : public GraphicsObject
{
public:
	explicit ConstantBuffer(Graphics* pGraphics);
	~ConstantBuffer();

	DELETE_COPY(ConstantBuffer)

	void SetSize(unsigned int size);
	void Apply();
	bool SetParameter(unsigned int offset, unsigned int size, const void* pData);
	bool IsDirty() const { return m_IsDirty; }
	void Release();
	int GetSize() const { return m_Size; }

private:
	bool m_IsDirty = false;
	unsigned int m_Size = 0;
	unsigned char* m_pShadowData = nullptr;
};