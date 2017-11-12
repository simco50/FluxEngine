#include "stdafx.h"
#include "Renderer.h"
#include "Drawable.h"
#include "Material.h"
#include "Rendering/Core/Graphics.h"
#include "Geometry.h"
#include "Core/ShaderVariation.h"
#include "Camera/Camera.h"

Renderer::Renderer(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{

}

Renderer::~Renderer()
{

}

void Renderer::Draw()
{
	for (Camera* pCamera : m_Cameras)
	{
		if (pCamera == nullptr)
			continue;

		m_pGraphics->SetViewport(pCamera->GetViewport(), false);
		for (Drawable* pDrawable : m_Drawables)
		{
			if (pDrawable == nullptr)
				continue;

			const vector<Batch>& batches = pDrawable->GetBatches();
			for (const Batch& batch : batches)
			{
				if (batch.pGeometry)
				{
					SetMaterial(batch.pMaterial);

					batch.pGeometry->Draw(m_pGraphics);
				}
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

void Renderer::SetMaterial(const Material* pMaterial)
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
}