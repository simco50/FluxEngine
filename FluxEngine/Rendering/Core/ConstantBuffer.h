#pragma once
class ConstantBuffer
{
public:
	ConstantBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
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

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
};