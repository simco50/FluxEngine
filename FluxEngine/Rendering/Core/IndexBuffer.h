#pragma once
class IndexBuffer
{
public:
	IndexBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~IndexBuffer();

	void Create(const int indexCount, bool dynamic = false);

	ID3D11Buffer* GetBuffer() const { return m_pBuffer.Get(); }

	void* Map(bool discard);
	void Unmap();

private:
	Smart_COM::Unique_COM<ID3D11Buffer> m_pBuffer;

	bool m_Dynamic = false;
	int m_IndexCount = -1;
	bool m_HardwareLocked = false;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
};

