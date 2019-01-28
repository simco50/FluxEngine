#include "FluxEngine.h"
#include "Light.h"
#include "Renderer.h"
#include "Core/TextureCube.h"
#include "Core/Texture2D.h"
#include "Core/Texture.h"
#include "Scenegraph/SceneNode.h"

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
	OnMarkedDirty(m_pNode);
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
			static_cast<TextureCube*>(m_pShadowTexture.get())->SetSize(512, 512, DXGI_FORMAT_R32_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, 1, nullptr);
		}
		else if ((m_pShadowTexture == nullptr || m_pShadowTexture->IsTypeOf<TextureCube>()) && m_Data.Type != Type::Point)
		{
			//need Texture2D
			m_pShadowTexture = std::make_unique<Texture2D>(m_pContext);
			static_cast<Texture2D*>(m_pShadowTexture.get())->SetSize(512, 512, DXGI_FORMAT_R32_TYPELESS, TextureUsage::DEPTHSTENCILBUFFER, 1, nullptr);
		};
	}
	else
	{
		m_pShadowTexture.reset();
	}
}

void Light::OnSceneSet(Scene* /*pScene*/)
{
	GetSubsystem<Renderer>()->AddLight(this);
	OnMarkedDirty(m_pNode);
}

void Light::OnSceneRemoved()
{
	GetSubsystem<Renderer>()->RemoveLight(this);
}

void Light::OnMarkedDirty(const SceneNode* pNode)
{
	m_Data.Position = pNode->GetWorldPosition();
	m_Data.Direction = pNode->GetForward();

	Matrix projection;
	switch (m_Data.Type)
	{
	case Type::Directional:
		projection = Math::CreateOrthographicMatrix(512, 512, 0.1f, 1000);
		break;
	case Type::Point:
		projection = Math::CreatePerspectiveMatrix(Math::PI, 1.0f, 0.1f, 1000);
		break;
	case Type::Spot:
		projection = Math::CreatePerspectiveMatrix(m_Data.SpotLightAngle * (Math::PI / 180.0f), 1.0f, 0.1f, 1000);
		break;
	default:
		break;
	}
	m_ViewProjection = pNode->GetWorldMatrix() * projection;
}

void Light::CreateUI()
{
	ImGui::Checkbox("Enabled", (bool*)&m_Data.Enabled);
	ImGui::Combo("Type", (int*)&m_Data.Type, [](void*, int selected, const char** pName)
	{
		Light::Type type = (Light::Type)selected;
		switch (type)
		{
		case Light::Type::Directional:
			*pName = "Directional";
			break;
		case Light::Type::Point:
			*pName = "Point";
			break;
		case Light::Type::Spot:
			*pName = "Spot";
			break;
		default:
			break;
		}
		return true;
	}, nullptr, (int)Light::Type::MAX);
	if (ImGui::InputFloat3("Position", &m_Data.Position.x))
	{
		m_pNode->SetPosition(m_Data.Position, Space::World);
	}
	m_Data.Direction.Normalize();
	if (ImGui::SliderFloat3("Direction", &m_Data.Direction.x, -1, 1))
	{
		m_pNode->LookInDirection(m_Data.Direction);
	}

	ImGui::ColorEdit4("Color", &m_Data.Colour.x);
	ImGui::SliderFloat("Intensity", &m_Data.Intensity, 0, 3);
	ImGui::SliderFloat("Range", &m_Data.Range, 0, 1000);
	ImGui::SliderFloat("SpotLightAngle", &m_Data.SpotLightAngle, 0, 90);
	ImGui::SliderFloat("Attenuation", &m_Data.Attenuation, 0, 1);
}
