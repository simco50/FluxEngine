#pragma once
class IndexBuffer
{
public:
	IndexBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~IndexBuffer();

	DELETE_COPY(IndexBuffer)

	void Create(const int indexCount, bool dynamic = false);
	void SetData(void* pData);

	void* GetBuffer() const { return m_pBuffer; }

	void* Map(bool discard);
	void Unmap();

private:
	void Release();

	void* m_pBuffer = nullptr;

	bool m_Dynamic = false;
	int m_IndexCount = -1;
	bool m_HardwareLocked = false;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
};

