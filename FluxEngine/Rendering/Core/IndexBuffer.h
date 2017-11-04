#pragma once
class Graphics;

class IndexBuffer
{
public:
	IndexBuffer(Graphics* pGraphics);
	~IndexBuffer();

	DELETE_COPY(IndexBuffer)

	void Create(const int indexCount, const bool smallIndexStride = false, const bool dynamic = false);
	void SetData(void* pData);

	void* GetBuffer() const { return m_pBuffer; }

	void* Map(bool discard);
	void Unmap();

	unsigned int GetCount() const { return m_IndexCount; }
	bool IsSmallStride() const { return m_SmallIndexStride; }

private:
	void* m_pBuffer = nullptr;

	bool m_Dynamic = false;
	unsigned int m_IndexCount = 0;
	bool m_SmallIndexStride = false;
	bool m_HardwareLocked = false;

	Graphics* m_pGraphics;
};

