#include "FluxEngine.h"
#include "Renderer.h"
#include "Drawable.h"
#include "Material.h"
#include "Rendering/Core/Graphics.h"
#include "Geometry.h"
#include "Core/ShaderVariation.h"
#include "Camera/Camera.h"
#include "Rendering/Core/Texture.h"
#include "Core/BlendState.h"
#include "Core/RasterizerState.h"
#include "Core/DepthStencilState.h"
#include "ParticleSystem/ParticleEmitter.h"

Renderer::Renderer(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{

}

Renderer::~Renderer()
{

}

void Renderer::Draw()
{
	m_pCurrentMaterial = nullptr;
	m_pCurrentCamera = nullptr;

	for (Camera* pCamera : m_Cameras)
	{
		if (pCamera == nullptr)
			continue;
		
		m_pGraphics->SetViewport(pCamera->GetViewport(), false);

		for (Drawable* pDrawable : m_Drawables)
		{
			if (pDrawable == nullptr)
				continue;
			if (!pDrawable->DrawEnabled())
				continue;
			if(!pCamera->GetFrustum().Intersects(pDrawable->GetWorldBoundingBox()))
				continue;

			ParticleEmitter* emitter = DynamicCast<ParticleEmitter>(pDrawable);
			emitter;
			const vector<Batch>& batches = pDrawable->GetBatches();
			for (const Batch& batch : batches)
			{
				if (batch.pGeometry == nullptr)
					continue;

				SetPerMaterialParameters(batch.pMaterial);
				SetPerBatchParameters(batch, pCamera);
				SetPerFrameParameters();
				SetPerCameraParameters(pCamera);
				batch.pGeometry->Draw(m_pGraphics);
			}
		}
	}
}

void Renderer::AddDrawable(Drawable* pDrawable)
{
	for (Drawable*& pD : m_Drawables)
	{
		if (pD == nullptr)
		{
			pD = pDrawable;
			return;
		}
	}
	m_Drawables.push_back(pDrawable);
}

bool Renderer::RemoveDrawable(Drawable* pDrawable)
{
	for (Drawable*& pD : m_Drawables)
	{
		if (pD == pDrawable)
		{
			pD = nullptr;
			return true;
		}
	}
	return false;
}

void Renderer::AddCamera(Camera* pCamera)
{
	for (Camera*& pC : m_Cameras)
	{
		if (pC == nullptr)
		{
			pC = pCamera;
			return;
		}
	}
	m_Cameras.push_back(pCamera);
}

bool Renderer::RemoveCamera(Camera* pCamera)
{
	for (Camera*& pC : m_Cameras)
	{
		if (pC == pCamera)
		{
			pC = nullptr;
			return true;
		}
	}
	return false;
}

void Renderer::SetPerFrameParameters()
{
	int frame = GameTimer::Ticks();
	if (frame != m_CurrentFrame)
	{
		m_CurrentFrame = frame;
		float deltaTime = GameTimer::DeltaTime();
		m_pGraphics->SetShaderParameter("cDeltaTime", &deltaTime);
		m_pGraphics->SetShaderParameter("cElapsedTime", &deltaTime);
		m_pGraphics->SetShaderParameter("cLightDirection", &m_LightDirection);
	}
}

void Renderer::SetPerCameraParameters(Camera* pCamera)
{
	if (m_pCurrentCamera != pCamera)
	{
		m_pCurrentCamera = pCamera;
		Matrix viewProj = pCamera->GetViewProjection();
		Matrix view = pCamera->GetView();
		Matrix viewInv = pCamera->GetViewInverse();
		float nearPlane = pCamera->GetNearPlane();
		float farPlane = pCamera->GetFarPlane();

		m_pGraphics->SetShaderParameter("cViewProj", &viewProj);
		m_pGraphics->SetShaderParameter("cView", &view);
		m_pGraphics->SetShaderParameter("cViewInverse", &viewInv);
		m_pGraphics->SetShaderParameter("cNearClip", &nearPlane);
		m_pGraphics->SetShaderParameter("cFarClip", &farPlane);
	}
}

void Renderer::SetPerMaterialParameters(const Material* pMaterial)
{
	if (pMaterial == nullptr)
	{
		//Set the default material
	}

	if (pMaterial == m_pCurrentMaterial)
		return;

	m_pCurrentMaterial = pMaterial;

	m_pGraphics->SetShader(ShaderType::VertexShader, pMaterial->GetShader(ShaderType::VertexShader));
	m_pGraphics->SetShader(ShaderType::PixelShader, pMaterial->GetShader(ShaderType::PixelShader));
	m_pGraphics->SetShader(ShaderType::GeometryShader, pMaterial->GetShader(ShaderType::GeometryShader));
	m_pGraphics->SetShader(ShaderType::ComputeShader, pMaterial->GetShader(ShaderType::ComputeShader));

	const auto& pParameters = pMaterial->GetShaderParameters();
	const auto& pParameterBuffer = pMaterial->GetParameterBuffer();
	for (const auto& pParameter : pParameters)
	{
		m_pGraphics->SetShaderParameter(pParameter.first, &pParameterBuffer[pParameter.second]);
	}

	const auto& pTextures = pMaterial->GetTextures();
	for (const auto& pTexture : pTextures)
	{
		m_pGraphics->SetTexture(pTexture.first, pTexture.second);
	}

	//Blend state
	m_pGraphics->GetBlendState()->SetBlendMode(pMaterial->GetBlendMode(), pMaterial->GetAlphaToCoverage());
	
	//Rasterizer state
	m_pGraphics->GetRasterizerState()->SetCullMode(pMaterial->GetCullMode());
	m_pGraphics->GetRasterizerState()->SetFillMode(pMaterial->GetFillMode());
	
	//Depth stencil state
	m_pGraphics->GetDepthStencilState()->SetDepthTest(pMaterial->GetDepthTestMode());
	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(pMaterial->GetDepthEnabled());
}

void Renderer::SetPerBatchParameters(const Batch& batch, Camera* pCamera)
{
	m_pGraphics->SetShaderParameter("cWorld", batch.pModelMatrix);
	Matrix wvp = *batch.pModelMatrix * pCamera->GetViewProjection();
	m_pGraphics->SetShaderParameter("cWorldViewProj", &wvp);
}