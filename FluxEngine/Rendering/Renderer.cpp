#include "stdafx.h"
#include "Renderer.h"
#include "Drawable.h"
#include "Material.h"
#include "Rendering/Core/Graphics.h"
#include "Geometry.h"
#include "Core/ShaderVariation.h"

Renderer::Renderer(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{

}

Renderer::~Renderer()
{

}

void Renderer::Draw()
{
	for (Drawable* pDrawable : m_Drawables)
	{
		const vector<Batch>& batches = pDrawable->GetBatches();
		for (const Batch& batch : batches)
		{
			SetMaterial(batch.pMaterial);
			batch.pGeometry->Draw(m_pGraphics);
		}
	}
}

void Renderer::AddDrawable(Drawable* pDrawable)
{
	m_Drawables.push_back(pDrawable);
}

void Renderer::SetMaterial(Material* pMaterial)
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