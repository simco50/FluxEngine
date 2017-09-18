#pragma once
class ConstantBuffer
{
public:
	ConstantBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~ConstantBuffer();

	void SetSize(const unsigned int size);
	void Apply();
	void SetParameter(unsigned char offset, const unsigned char size, const void* pData);
	bool IsDirty() const { return m_IsDirty; }
	void Release();
	int GetSize() const { return m_Size; }

private:
	bool m_IsDirty = false;
	ID3D11Buffer* m_pBuffer;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

	unsigned int m_Size = 0;

	unsigned char* m_pShadowData = nullptr;
};