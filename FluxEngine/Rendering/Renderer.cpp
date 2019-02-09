#include "FluxEngine.h"
#include "Renderer.h"
#include "Async/AsyncTaskQueue.h"
#include "Camera/Camera.h"
#include "Core/Graphics.h"
#include "Core/IndexBuffer.h"
#include "Core/VertexBuffer.h"
#include "Drawable.h"
#include "Geometry.h"
#include "Light.h"
#include "Material.h"
#include "Scenegraph/SceneNode.h"
#include "Math/DualQuaternion.h"
#include "PostProcessing.h"
#include "Core/RenderTarget.h"
#include "Content/ResourceManager.h"
#include "Core/Texture.h"
#include "ParticleSystem/ParticleSystem.h"
#include "ParticleSystem/ParticleEmitter.h"
#include "Mesh.h"
#include "Model.h"
#include "ReflectionProbe.h"
#include "Animation/AnimatedModel.h"

Renderer::Renderer(Context* pContext) :
	Subsystem(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
	CreateQuadGeometry();
	m_pBlitMaterial = pContext->GetSubsystem<ResourceManager>()->Load<Material>("Materials/Blit.xml");

	pContext->RegisterFactory<Light>();
	pContext->RegisterFactory<Camera>();
	pContext->RegisterFactory<Material>();
	pContext->RegisterFactory<ParticleSystem>();
	pContext->RegisterFactory<ParticleEmitter>();
	pContext->RegisterFactory<Mesh>();
	pContext->RegisterFactory<Drawable>();
	pContext->RegisterFactory<Model>();
	pContext->RegisterFactory<PostProcessing>();
	pContext->RegisterFactory<ReflectionProbe>();
	pContext->RegisterFactory<AnimatedModel>();
}

Renderer::~Renderer()
{

}

void Renderer::Draw()
{
	AUTOPROFILE(Renderer_Draw);

	m_pCurrentMaterial = nullptr;

	{
		AUTOPROFILE(Renderer_UpdateDrawables);
		for(size_t i = 0; i < m_Drawables.size(); ++i)
		{
			AUTOPROFILE(Renderer_UpdateDrawable);
			m_Drawables[i]->Update();
		}
	}

	{
		AUTOPROFILE(Renderer_PreRender);
		m_OnPreRender.Broadcast();
	}

	std::vector<const View*> pViews = m_ViewQueue;
	m_ViewQueue.clear();
	for (Camera* pCamera : m_Cameras)
	{
		if (pCamera->IsEnabled())
		{
			pViews.push_back(&pCamera->GetViewData());
		}
	}

	{
		AUTOPROFILE(Renderer_Draw_All);

		for (const View* pView : pViews)
		{
			AUTOPROFILE(Renderer_Draw_Camera);

			if (pView == nullptr)
			{
				continue;
			}

			m_pGraphics->SetRenderTarget(0, pView->pRenderTarget);
			m_pGraphics->SetDepthStencil(pView->pDepthStencil);
			m_pGraphics->SetViewport(pView->Viewport.Scale((float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight()));
			m_pGraphics->Clear(pView->ClearFlags, pView->ClearColor, 1.0f, 1);

			m_pCurrentMaterial = nullptr;

			for (Drawable* pDrawable : m_Drawables)
			{
				AUTOPROFILE_DESC(Renderer_Draw_Drawable, pDrawable->GetNode()->GetName());

				if (pDrawable == nullptr)
				{
					continue;
				}
				if (pDrawable->GetCullingEnabled() && !pView->Frustum.Intersects(pDrawable->GetWorldBoundingBox()))
				{
					continue;
				}

				const std::vector<Batch>& batches = pDrawable->GetBatches();
				for (const Batch& batch : batches)
				{
					if (batch.pGeometry == nullptr || batch.pMaterial == nullptr)
					{
						continue;
					}

					SetPerMaterialParameters(batch.pMaterial);
					SetPerBatchParameters(batch, pView);
					SetPerFrameParameters();
					SetPerViewParameters(pView);

					batch.pGeometry->Draw(m_pGraphics);
				}
			}
		}
	}

	for (PostProcessing* pPost : m_PostProcessing)
	{
		if (pPost->IsEnabled())
		{
			pPost->Draw();
		}
	}
}

void Renderer::AddDrawable(Drawable* pDrawable)
{
	m_Drawables.push_back(pDrawable);
}

void Renderer::RemoveDrawable(Drawable* pDrawable)
{
	m_Drawables.erase(std::remove(m_Drawables.begin(), m_Drawables.end(), pDrawable), m_Drawables.end());
}

void Renderer::AddCamera(Camera* pCamera)
{
	m_Cameras.push_back(pCamera);
	std::sort(m_Cameras.begin(), m_Cameras.end(), [](Camera* pA, Camera* pB) {return pA->GetRenderOrder() > pB->GetRenderOrder(); });
}

void Renderer::RemoveCamera(Camera* pCamera)
{
	m_Cameras.erase(std::remove(m_Cameras.begin(), m_Cameras.end(), pCamera), m_Cameras.end());
}

void Renderer::AddLight(Light* pLight)
{
	m_Lights.push_back(pLight);
}

void Renderer::RemoveLight(Light* pLight)
{
	m_Lights.erase(std::remove(m_Lights.begin(), m_Lights.end(), pLight), m_Lights.end());
}

void Renderer::AddPostProcessing(PostProcessing* pPostProcessing)
{
	m_PostProcessing.push_back(pPostProcessing);
}

void Renderer::RemovePostProcessing(PostProcessing* pPostProcessing)
{
	m_PostProcessing.erase(std::remove(m_PostProcessing.begin(), m_PostProcessing.end(), pPostProcessing), m_PostProcessing.end());
}

void Renderer::Blit(RenderTarget* pSource, RenderTarget* pTarget, Material* pMaterial /*= nullptr*/)
{
	check(pSource);
	check(pTarget);
	check(pSource->GetParentTexture()->GetWidth() == pTarget->GetParentTexture()->GetWidth());
	check(pSource->GetParentTexture()->GetHeight() == pTarget->GetParentTexture()->GetHeight());
	if (pMaterial == nullptr)
	{
		pMaterial = m_pBlitMaterial;
	}
	m_pGraphics->SetTexture(TextureSlot::Diffuse, nullptr);
	m_pGraphics->FlushSRVChanges(false);
	m_pGraphics->SetTexture(TextureSlot::Diffuse, pSource->GetParentTexture());
	m_pGraphics->SetRenderTarget(0, pTarget);
	SetPerMaterialParameters(pMaterial);
	m_pGraphics->GetPipelineState()->SetDepthEnabled(false);
	m_pGraphics->GetPipelineState()->SetDepthWrite(false);
	GetQuadGeometry()->Draw(m_pGraphics);
}

void Renderer::QueueView(const View* pView)
{
	m_ViewQueue.push_back(pView);
}

void Renderer::CreateQuadGeometry()
{
	AUTOPROFILE(Renderer_CreateQuadGeometry);

	struct VertexStructure
	{
		Vector3 Position;
		Vector2 TexCoord;
	};

	m_pQuadVertexBuffer = std::make_unique<VertexBuffer>(m_pGraphics);
	std::vector<VertexElement> elements = {
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION, 0, false),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD, 0, false),
	};
	m_pQuadVertexBuffer->Create(4, elements);
	VertexStructure vertices[] = {
		{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(0, 0) },
		{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1, 0) },
		{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1, 1) },
		{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(0, 1) },
	};
	m_pQuadVertexBuffer->SetData(vertices);

	m_pQuadIndexBuffer = std::make_unique<IndexBuffer>(m_pGraphics);
	m_pQuadIndexBuffer->Create(6, false, false);
	int indices[] = {
		0, 1, 2,
		0, 2, 3
	};
	m_pQuadIndexBuffer->SetData(indices);

	m_pQuadGeometry = std::make_unique<Geometry>();
	m_pQuadGeometry->SetIndexBuffer(m_pQuadIndexBuffer.get());
	m_pQuadGeometry->SetVertexBuffer(m_pQuadVertexBuffer.get());
	m_pQuadGeometry->SetDrawRange(PrimitiveType::TRIANGLELIST, 6, 4);
}

void Renderer::SetPerFrameParameters()
{
	m_pGraphics->SetShaderParameter(ShaderConstant::cDeltaTime, GameTimer::DeltaTime());
	m_pGraphics->SetShaderParameter(ShaderConstant::cElapsedTime, GameTimer::GameTime());
}

void Renderer::SetPerViewParameters(const View* pView)
{
	m_pGraphics->SetShaderParameter(ShaderConstant::cView, pView->ViewMatrix);
	m_pGraphics->SetShaderParameter(ShaderConstant::cViewProj, pView->ViewProjectionMatrix);
	m_pGraphics->SetShaderParameter(ShaderConstant::cViewInverse, pView->ViewInverseMatrix);
	m_pGraphics->SetShaderParameter(ShaderConstant::cNearClip, pView->NearPlane);
	m_pGraphics->SetShaderParameter(ShaderConstant::cFarClip, pView->FarPlane);
}

void Renderer::SetPerMaterialParameters(const Material* pMaterial)
{
	if (pMaterial == m_pCurrentMaterial)
	{
		return;
	}
	m_pCurrentMaterial = pMaterial;

	for (int i = 0; i < (int)ShaderType::MAX; ++i)
	{
		m_pGraphics->SetShader((ShaderType)i, m_pCurrentMaterial->GetShader((ShaderType)i));
	}

	const auto& pParameters = m_pCurrentMaterial->GetShaderParameters();
	for (const auto& pParameter : pParameters)
	{
		m_pGraphics->SetShaderParameter(pParameter.first, pParameter.second.GetData());
	}

	const auto& pTextures = m_pCurrentMaterial->GetTextures();
	for (const auto& pTexture : pTextures)
	{
		m_pGraphics->SetTexture(pTexture.first, pTexture.second);
	}

	std::vector<Light::Data> lightData(GraphicsConstants::MAX_LIGHTS);
	for (size_t i = 0; i < m_Lights.size(); ++i)
	{
		lightData[i] = *m_Lights[i]->GetData();
	}
	m_pGraphics->SetShaderParameter(ShaderConstant::cLights, lightData.data());

	//Blend state
	m_pGraphics->GetPipelineState()->SetBlendMode(m_pCurrentMaterial->GetBlendMode(), m_pCurrentMaterial->GetAlphaToCoverage());

	//Rasterizer state
	m_pGraphics->GetPipelineState()->SetCullMode(m_pCurrentMaterial->GetCullMode());
	m_pGraphics->GetPipelineState()->SetFillMode(m_pCurrentMaterial->GetFillMode());

	//Depth stencil state
	m_pGraphics->GetPipelineState()->SetDepthTest(m_pCurrentMaterial->GetDepthTestMode());
	m_pGraphics->GetPipelineState()->SetDepthEnabled(m_pCurrentMaterial->GetDepthEnabled());
	m_pGraphics->GetPipelineState()->SetDepthWrite(m_pCurrentMaterial->GetDepthWrite());
}

void Renderer::SetPerBatchParameters(const Batch& batch, const View* pView)
{
	if (batch.NumSkinMatrices > 0)
	{
		m_pGraphics->SetShaderParameter(ShaderConstant::cSkinMatrices, batch.pWorldMatrices, sizeof(Matrix), batch.NumSkinMatrices);
		m_pGraphics->SetShaderParameter(ShaderConstant::cSkinDualQuaternions, batch.pSkinDualQuaternions, sizeof(DualQuaternion), batch.NumSkinMatrices);
	}
	else
	{
		m_pGraphics->SetShaderParameter(ShaderConstant::cWorld, *batch.pWorldMatrices);
		Matrix wvp = *batch.pWorldMatrices * pView->ViewProjectionMatrix;
		m_pGraphics->SetShaderParameter(ShaderConstant::cWorldViewProj, wvp);
	}
}