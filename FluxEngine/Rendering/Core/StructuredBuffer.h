#pragma once

class Graphics;

class StructuredBuffer
{
public:
	StructuredBuffer(Graphics* pGraphics);
	~StructuredBuffer();

	DELETE_COPY(StructuredBuffer)

	void Create(const int elementCount, const int elementStride, bool dynamic = false);
	void SetData(void* pData);

	template<typename T>
	void SetElement(const size_t index, const T& element)
	{
		assert(sizeof(T) == m_Stride);
		SetElement_Internal(index, &element);
	}

	void* GetBuffer() const { return m_pBuffer; }
	void* GetView() const { return m_pShaderResourceView; }

	void* Map(bool discard);
	void Unmap();
	void* GetMappedData() const { return m_pMappedData; }

	unsigned int GetVertexStride() const { return m_Stride; }
	unsigned int GetElementCount() const { return m_ElementCount; }

private:
	void SetElement_Internal(const size_t index, const void* pElement);

	void Release();

	void* m_pBuffer = nullptr;
	void* m_pShaderResourceView = nullptr;

	bool m_Dynamic = false;
	bool m_Mapped = false;
	void* m_pMappedData = nullptr;

	unsigned int m_ElementCount = 0;
	unsigned int m_Stride = 0;

	Graphics* m_pGraphics;
};
