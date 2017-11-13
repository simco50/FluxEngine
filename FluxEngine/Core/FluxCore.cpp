#include "stdafx.h"
#include "FluxCore.h"
#include "UI/ImmediateUI.h"
#include "Rendering/Core/Graphics.h"
#include "Rendering/Core/ShaderVariation.h"
#include "Rendering/Core/Shader.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Rendering/Core/ConstantBuffer.h"
#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/Mesh.h"
#include "Rendering/Camera/FreeCamera.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Core/RasterizerState.h"
#include "Rendering/Core/BlendState.h"
#include "Rendering/Core/DepthStencilState.h"
#include "Config.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/Core/RenderTarget.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Geometry.h"
#include "Scenegraph/Scene.h"
#include "Rendering/Model.h"
#include "Audio/AudioSource.h"

using namespace std;

FluxCore::FluxCore()
{
}

FluxCore::~FluxCore()
{
	SafeDelete(m_pGraphics);

	Console::Release();
	Config::Flush();
}

int FluxCore::Run(HINSTANCE hInstance)
{
	AUTOPROFILE(FluxCore_Run);
	{
		AUTOPROFILE(FluxCore_Initialize);

		Console::Initialize();

#ifdef NDEBUG
		if (!FileSystem::Mount("./Resources.pak", "Resources", ArchiveType::Pak))
		{
			FLUX_LOG(WARNING, "Failed to mount './Resources.paK'");
	}
#else
		FLUX_LOG(WARNING, "[DEBUG] Skipped mounting './Resources.paK'");
#endif
		if (!FileSystem::Mount("./Resources", "Resources", ArchiveType::Physical))
		{
			FLUX_LOG(WARNING, "Failed to mount './Resources'");
		}

		Config::Initialize();

		m_pGraphics = new Graphics(hInstance);
		if (!m_pGraphics->SetMode(
			Config::GetInt("Width", "Window", 1240),
			Config::GetInt("Height", "Window", 720),
			(WindowType)Config::GetInt("WindowMode", "Window", 0),
			Config::GetBool("Resizable", "Window", 1),
			Config::GetBool("VSync", "Window", 1),
			Config::GetInt("MSAA", "Window", 8),
			Config::GetInt("RefreshRate", "Window", 60)))
		{
			FLUX_LOG(ERROR, "[FluxCore::Run] > Failed to initialize graphics");
		}

		m_pGraphics->SetWindowTitle(Config::GetString("Name", "Game", "FluxEngine"));
		m_pInput = make_unique<InputEngine>(m_pGraphics);
		m_pImmediateUI = make_unique<ImmediateUI>(m_pGraphics, m_pInput.get());

		GameTimer::Reset();

		InitGame();
	}

	//Game loop
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (!GameTimer::IsPaused())
			{
				GameTimer::Tick();
				GameLoop();
			}
			else
				Sleep(100);
		}
	}
	return (int)msg.wParam;
}

void FluxCore::InitGame()
{
	AUTOPROFILE(FluxCore_InitGame);

	m_pShader = m_pGraphics->GetShader("Resources/Shaders/Diffuse.hlsl");
	if (m_pShader)
	{
		m_pVertexShader = m_pShader->GetVariation(ShaderType::VertexShader);
		m_pPixelShader = m_pShader->GetVariation(ShaderType::PixelShader);
	}

	//MeshFilter
	vector<VertexElement> elements;
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::POSITION });
	elements.push_back({ VertexElementType::VECTOR2, VertexElementSemantic::TEXCOORD });
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::NORMAL });

	m_pScene = make_unique<Scene>(m_pGraphics);
	SceneNode* pNode = new SceneNode();
	m_pMeshFilter = make_unique<Mesh>();
	m_pMeshFilter->Load("Resources/Meshes/Spot.flux");
	m_pMeshFilter->CreateBuffers(m_pGraphics, elements);
	Model* pModel = new Model();
	pModel->SetMesh(m_pMeshFilter.get());
	pNode->AddComponent(pModel);
	m_pScene->AddChild(pNode);
	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, 1, 1), true);

	m_pCamera = new FreeCamera(m_pInput.get(), m_pGraphics);
	m_pScene->AddChild(m_pCamera);

	//Texture
	m_pDiffuseTexture = make_unique<Texture>(m_pGraphics);
	m_pDiffuseTexture->Load("Resources/Textures/Spot.png");
}

void FluxCore::GameLoop()
{
	m_pInput->Update();
	m_pCamera->Update();

	m_pGraphics->BeginFrame();
	m_pGraphics->Clear(ClearFlags::All, Color(0.2f, 0.2f, 0.2f, 1.0f), 1.0f, 1);

	UpdatePerFrameParameters();
	UpdatePerViewParameters();
	UpdatePerObjectParameters();

	m_pGraphics->SetTexture(TextureSlot::Diffuse, m_pDiffuseTexture.get());

	m_pGraphics->SetShader(ShaderType::VertexShader, m_pVertexShader);
	m_pGraphics->SetShader(ShaderType::PixelShader, m_pPixelShader);
	m_pGraphics->SetScissorRect(false);
	m_pGraphics->GetRasterizerState()->SetCullMode(CullMode::BACK);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::REPLACE, false);
	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::LESS);

	m_pCamera->GetCamera()->SetViewport(0, 0, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight());
	m_pScene->Update();

	RenderUI();

	m_pGraphics->EndFrame();
}

void FluxCore::UpdatePerFrameParameters()
{
	m_pVertexShader->SetParameter("cDeltaTimeVS", GameTimer::DeltaTime());
	m_pVertexShader->SetParameter("cElapsedTimeVS", GameTimer::GameTime());

	m_pPixelShader->SetParameter("cDeltaTimePS", GameTimer::DeltaTime());
	m_pPixelShader->SetParameter("cLightDirectionPS", m_LightDirection);
}

void FluxCore::UpdatePerObjectParameters()
{
	Matrix viewProj = m_pCamera->GetCamera()->GetViewProjection();
	Matrix world = Matrix::CreateTranslation(0, 0, 3);
	m_pVertexShader->SetParameter("cWorldVS", world);
	m_pVertexShader->SetParameter("cWorldViewProjVS", world * viewProj);

	m_pPixelShader->SetParameter("cColorPS", m_Color);
	m_pPixelShader->SetParameter("cWorldPS", world);
}

void FluxCore::UpdatePerViewParameters()
{
	m_pVertexShader->SetParameter("cViewProjVS", m_pCamera->GetCamera()->GetViewProjection());
	m_pVertexShader->SetParameter("cViewVS", m_pCamera->GetCamera()->GetView());
	m_pVertexShader->SetParameter("cViewInverseVS", m_pCamera->GetCamera()->GetViewInverse());
	m_pVertexShader->SetParameter("cNearClipVS", m_pCamera->GetCamera()->GetNearPlane());
	m_pVertexShader->SetParameter("cFarClipVS", m_pCamera->GetCamera()->GetFarPlane());

	m_pVertexShader->SetParameter("cViewProjPS", m_pCamera->GetCamera()->GetViewProjection());
	m_pVertexShader->SetParameter("cViewPS", m_pCamera->GetCamera()->GetView());
	m_pVertexShader->SetParameter("cViewInversePS", m_pCamera->GetCamera()->GetViewInverse());
}

void FluxCore::RenderUI()
{
	m_pImmediateUI->NewFrame();

	unsigned int batchCount, primitiveCount;
	m_pGraphics->GetDebugInfo(batchCount, primitiveCount);

	stringstream timeStr;
	timeStr << setw(2) << setfill('0') << (int)GameTimer::GameTime() / 60 << ":" << setw(2) << (int)GameTimer::GameTime() % 60;
	string time = timeStr.str();
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Game Time : %s", time.c_str());
	ImGui::Text("MS: %f", GameTimer::DeltaTime());
	ImGui::Text("FPS: %f", 1.0f / GameTimer::DeltaTime());
	ImGui::Text("Primitives: %i", primitiveCount);
	ImGui::Text("Batches: %i", batchCount);
	ImGui::End();

	ImGui::ColorPicker4("Color Picker", &m_Color.x);
	ImGui::SliderFloat3("Light Direction", &m_LightDirection.x, -1, 1);

	m_pImmediateUI->Render();
}