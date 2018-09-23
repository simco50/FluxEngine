#include "FluxEngine.h"
#include "Light.h"
#include "Renderer.h"
#include "Scenegraph/Transform.h"
#include "Core/TextureCube.h"
#include "Core/Texture2D.h"
#include "Core/Texture.h"

Light::Light(Context* pContext)
	: Component(pContext)
{
}

Light::~Light()
{

}

void Light::SetType(const Type type)
{
	m_Data.Type = type;
	SetShadowCasting(m_CastShadow);
	OnMarkedDirty(GetTransform());
}

void Light::SetShadowCasting(const bool enabled)
{
	m_CastShadow = enabled;
	if (enabled)
	{
		if ((m_pShadowTexture == nullptr || m_pShadowTexture->IsTypeOf<Texture2D>()) && m_Data.Type == Type::Point)
		{
			//need TextureCube
			m_pShadowTexture = std::make_unique<TextureCube>(m_pContext);
			((TextureCube*)m_pShadowTexture.get())->SetSize(512, 512, DXGI_FORMAT_R32_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, 1, nullptr);
		}
		else if ((m_pShadowTexture == nullptr || m_pShadowTexture->IsTypeOf<TextureCube>()) && m_Data.Type != Type::Point)
		{
			//need Texture2D
			m_pShadowTexture = std::make_unique<Texture2D>(m_pContext);
			((Texture2D*)m_pShadowTexture.get())->SetSize(512, 512, DXGI_FORMAT_R32_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, 1, nullptr);
		};
	}
	else
	{
		m_pShadowTexture.reset();
	}
}

void Light::OnSceneSet(Scene* /*pScene*/)
{
	m_pContext->GetSubsystem<Renderer>()->AddLight(this);
	OnMarkedDirty(GetTransform());
}

void Light::OnSceneRemoved()
{
	m_pContext->GetSubsystem<Renderer>()->RemoveLight(this);
}

void Light::OnMarkedDirty(const Transform* transform)
{
	m_Data.Position = transform->GetWorldPosition();
	m_Data.Direction = transform->GetForward();

	Matrix projection;
	switch (m_Data.Type)
	{
	case Type::Directional:
		projection = XMMatrixOrthographicLH(512, 512, 0.1f, 1000);
		break;
	case Type::Point:
		projection = XMMatrixPerspectiveFovLH(XM_PI, 1.0f, 0.1f, 1000);
		break;
	case Type::Spot:
		projection = XMMatrixPerspectiveFovLH(m_Data.SpotLightAngle * (XM_PI / 180.0f), 1.0f, 0.1f, 1000);
		break;
	default:
		break;
	}
	m_ViewProjection = transform->GetWorldMatrix() * projection;
}
