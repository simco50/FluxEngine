#include "stdafx.h"
#include "FluxCore.h"
#include "Config.h"
#include "UI/ImmediateUI.h"

#include "Scenegraph/Scene.h"
#include "Scenegraph/Transform.h"

#include "Rendering/Core/Graphics.h"
#include "Rendering/Core/ShaderVariation.h"
#include "Rendering/Core/Shader.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Rendering/Core/RasterizerState.h"
#include "Rendering/Core/BlendState.h"
#include "Rendering/Core/DepthStencilState.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Core/ConstantBuffer.h"
#include "Rendering/Mesh.h"
#include "Rendering/Model.h"
#include "Rendering/Material.h"
#include "Rendering/Camera/FreeCamera.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/ParticleSystem/ParticleEmitter.h"

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

		if (!FileSystem::Mount("./Resources.pak", "Resources", ArchiveType::Pak))
		{
			FLUX_LOG(WARNING, "Failed to mount './Resources.paK'");
		}
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
			Config::GetBool("Resizable", "Window", true),
			Config::GetBool("VSync", "Window", true),
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

	m_pScene = make_unique<Scene>(m_pGraphics);
	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, 1, 1), true);
	m_pCamera = new FreeCamera(m_pInput.get(), m_pGraphics);
	m_pScene->AddChild(m_pCamera);

	m_pNode = new SceneNode("Particles");

	m_pParticleSystem = ResourceManager::Instance().Load<ParticleSystem>("Resources/ParticleSystems/Lava.json");
	ParticleEmitter* pEmitter = new ParticleEmitter(m_pGraphics, m_pParticleSystem);
	m_pNode->AddComponent(pEmitter);
	m_pScene->AddChild(m_pNode);
}

void FluxCore::GameLoop()
{
	m_pInput->Update();
	m_pCamera->Update();

	m_pGraphics->BeginFrame();
	m_pGraphics->Clear(ClearFlags::All, Color(0.2f, 0.2f, 0.2f, 1.0f), 1.0f, 1);

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

	m_pImmediateUI->Render();
}