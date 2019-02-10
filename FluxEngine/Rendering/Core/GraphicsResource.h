#pragma once
#include "GraphicsObject.h"
#include "Misc/SimpleMemory.h"

class GraphicsResource : public GraphicsObject
{
	DELETE_COPY(GraphicsResource);

public:
	explicit GraphicsResource(Graphics* pGraphics);
	virtual ~GraphicsResource();

	void Release();

	int GetSize() const { return m_Size; }
	bool SetData(const void* pData, bool immediate = true);
	bool SetData(const void* pData, int offset, int size, bool immediate = true);
	bool SetElement(int index, const void* pData, int size, bool immediate = false);
	void Apply();
	void* Map(bool discard);
	void Unmap();
	inline void* GetMappedData() const { return m_pMappedData; }
	inline int GetElementStride() const { return m_ElementStride; }
	inline int GetElementCount() const { return m_ElementCount; }

#ifdef GRAPHICS_D3D11
	inline void* GetView() const { return m_pShaderResourceView; }
#elif defined(GRAPHICS_D3D12)
	inline size_t GetGPUHandle() const { return m_GpuHandle; }
#endif

protected:
	SimpleMemory m_ShadowData;
	union
	{
		void* m_pShaderResourceView = nullptr;
		size_t m_GpuHandle;
	};
	void* m_pMappedData = nullptr;
	bool m_IsDirty = false;
	int m_Size = 0;
	int m_ElementCount = 0;
	int m_ElementStride = 0;
	bool m_Dynamic = false;
	bool m_Mapped = false;

private:
	bool Apply_Internal(const void* pData, int offset, int size);
};