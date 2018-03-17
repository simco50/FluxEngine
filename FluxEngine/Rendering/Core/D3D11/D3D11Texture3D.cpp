#include "FluxEngine.h"
#include "../Texture3D.h"
#include "../Graphics.h"

#include "D3D11GraphicsImpl.h"
#include "Content/Image.h"

Texture3D::Texture3D(Context* pContext) :
	Texture(pContext)
{

}

Texture3D::~Texture3D()
{

}

bool Texture3D::Load(InputStream& inputStream)
{
	AUTOPROFILE(Texture3D_Load);

	m_pImage = std::make_unique<Image>(m_pContext);
	if (!m_pImage->LoadLUT(inputStream))
		return false;

	return SetData(m_pImage.get());
}

bool Texture3D::SetSize(const int width, const int height, const int depth, const unsigned int format, TextureUsage usage, const int multiSample, void* pTexture)
{
	AUTOPROFILE(Texture3D_SetSize);

	if (multiSample > 1 && usage != TextureUsage::DEPTHSTENCILBUFFER && usage != TextureUsage::RENDERTARGET)
	{
		FLUX_LOG(Error, "[Texture::SetSize()] > Multisampling is only supported for rendertarget or depth-stencil textures");
		return false;
	}

	Release();

	m_Width = width;
	m_Height = height;
	m_Depth = depth;
	m_TextureFormat = format;
	m_Usage = usage;
	m_MultiSample = multiSample;
	m_pResource = pTexture;

	if (!Create())
		return false;
	return true;
}

bool Texture3D::SetData(const void* pData)
{
	AUTOPROFILE(Texture3D_SetData);

	if (m_Usage == TextureUsage::STATIC)
	{
		D3D11_BOX box;
		box.back = m_Depth;
		box.front = 0;
		box.left = 0;
		box.top = 0;
		box.right = m_Width;
		box.bottom = m_Height;
		m_pGraphics->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)m_pResource, 0, &box, pData, m_Width * 4, m_Depth * m_Width * 4);
	}
	else
	{
		FLUX_LOG(Error, "[Texture::SetData()] > Not yet implemented!");
		return false;
	}
	return true;
}

bool Texture3D::SetData(Image * pImage)
{
	AUTOPROFILE(Texture3D_SetData_Image);

	if (!SetSize(pImage->GetWidth(), pImage->GetHeight(), pImage->GetDepth(), DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::STATIC, 1, nullptr))
		return false;
	return SetData(pImage->GetData());
}

bool Texture3D::Create()
{
	AUTOPROFILE(Texture3D_Create);

	D3D11_TEXTURE3D_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = m_Usage == TextureUsage::DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.Format = (DXGI_FORMAT)m_TextureFormat;
	desc.Height = m_Height;
	desc.Width = m_Width;
	desc.Depth = m_Depth;
	desc.MipLevels = 1; //#todo mip levels
	desc.MiscFlags = 0; //#todo D3D11_RESOURCE_MISC_GENERATE_MIPS
	desc.Usage = (m_Usage == TextureUsage::DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	if (m_pResource == nullptr)
	{
		HR(m_pGraphics->GetImpl()->GetDevice()->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D**)&m_pResource));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Format = (DXGI_FORMAT)GetSRVFormat(m_TextureFormat);

	HR(m_pGraphics->GetImpl()->GetDevice()->CreateShaderResourceView((ID3D11Texture3D*)m_pResource, &srvDesc, (ID3D11ShaderResourceView**)&m_pShaderResourceView));

	return true;
}
