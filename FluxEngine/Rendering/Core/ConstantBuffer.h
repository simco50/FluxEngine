#pragma once
class Graphics;

class ConstantBuffer
{
public:
	ConstantBuffer(Graphics* pGraphics);
	~ConstantBuffer();

	DELETE_COPY(ConstantBuffer)

	void SetSize(const unsigned int size);
	void Apply();
	bool SetParameter(unsigned int offset, const unsigned int size, const void* pData);
	bool IsDirty() const { return m_IsDirty; }
	void Release();
	int GetSize() const { return m_Size; }

	void* GetBuffer() const { return m_pBuffer; }

private:

	bool m_IsDirty = false;
	void* m_pBuffer = nullptr;
	unsigned int m_Size = 0;
	unsigned char* m_pShadowData = nullptr;

	Graphics* m_pGraphics;
};