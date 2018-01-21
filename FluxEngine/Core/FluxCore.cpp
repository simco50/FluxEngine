#include "FluxEngine.h"
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
#include "Physics/PhysX/PhysicsSystem.h"
#include "Physics/PhysX/PhysicsScene.h"
#include "Physics/PhysX/Rigidbody.h"
#include "Physics/PhysX/Collider.h"
#include "Window.h"
#include "Rendering/DebugRenderer.h"

using namespace std;

FluxCore::FluxCore()
{
}

FluxCore::~FluxCore()
{
	SafeDelete(m_pWindow);
	Console::Release();
	Config::Flush();
}

int FluxCore::Run(HINSTANCE hInstance)
{
	UNREFERENCED_PARAMETER(hInstance);

	AUTOPROFILE(FluxCore_Run);
	{
		AUTOPROFILE(FluxCore_Initialize);

		Console::Initialize();

		if (!FileSystem::Mount("./Resources.pak", "Resources", ArchiveType::Pak))
		{
			FLUX_LOG(WARNING, "Failed to mount './Resources.pak'");
		}
		if (!FileSystem::Mount("./Resources", "Resources", ArchiveType::Physical))
		{
			FLUX_LOG(WARNING, "Failed to mount './Resources'");
		}

		Config::Initialize();

		m_pWindow = new Window(
			Config::GetInt("Width", "Window", 1240),
			Config::GetInt("Height", "Window", 720),
			Config::GetString("Title", "Window", "FluxEngine"),
			(WindowType)Config::GetInt("WindowMode", "Window", 0),
			Config::GetBool("Resizable", "Window", true),
			"windowClass"
			);
		if (!m_pWindow->Open())
			return 1;
		m_pWindow->SetIcon("Logo.ico");
		m_pWindow->OnWindowStateChanged().AddRaw(this, &FluxCore::OnPause);

		m_pGraphics = RegisterSubsystem(make_unique<Graphics>(m_pWindow));
		if (!m_pGraphics->SetMode(
			Config::GetBool("VSync", "Window", true),
			Config::GetInt("MSAA", "Window", 8),
			Config::GetInt("RefreshRate", "Window", 60)))
		{
			FLUX_LOG(ERROR, "[FluxCore::Run] > Failed to initialize graphics");
		}

		m_pInput = RegisterSubsystem(make_unique<InputEngine>(m_pWindow));
		m_pImmediateUI = RegisterSubsystem(make_unique<ImmediateUI>(m_pGraphics, m_pWindow, m_pInput));
		m_pPhysics = RegisterSubsystem(make_unique<PhysicsSystem>(nullptr));

		m_pDebugRenderer = RegisterSubsystem(make_unique<DebugRenderer>(m_pGraphics));

		InitGame();
		GameTimer::Reset();
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
	m_pCamera = new FreeCamera(m_pInput, m_pGraphics);
	m_pScene->AddChild(m_pCamera);

	PxMaterial* pPhysMaterial = m_pPhysics->GetPhysics()->createMaterial(0.2f, 0.2f, 0.8f);

	Mesh* pMesh = ResourceManager::Instance().Load<Mesh>("Resources/Meshes/Spot.flux");
	std::vector<VertexElement> desc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL)
	};
	pMesh->CreateBuffers(m_pGraphics, desc);

	Material* pMaterial = ResourceManager::Instance().Load<Material>("Resources/Materials/Default.xml", m_pGraphics);

	for (int x = 0; x < 1; ++x)
	{
		for (int y = 0; y < 1; ++y)
		{
			for (int z = 0; z < 1; ++z)
			{
				m_pNode = new SceneNode(Printf("Cow %i - %i - %i", x, y, z));
				m_pNode->GetTransform()->Translate(x * 3.0f + RandF(-0.1f, 0.1f), y * 4.0f + 8 + RandF(-0.1f, 0.1f), z * 3.0f + RandF(-0.1f, 0.1f));
				Model* pModel = new Model();
				pModel->SetMesh(pMesh);
				pModel->SetMaterial(pMaterial);
				m_pNode->AddComponent(pModel);
				Rigidbody* pRigidbody = new Rigidbody(m_pPhysics);
				pRigidbody->SetBodyType(Rigidbody::Dynamic);
				m_pNode->AddComponent(pRigidbody);
				Collider* pCollider = new BoxCollider(m_pPhysics, pMesh->GetBoundingBox(), pPhysMaterial);
				m_pNode->AddComponent(pCollider);
				m_pScene->AddChild(m_pNode);
			}
		}
	}
	
	SceneNode* pFloor = new SceneNode("Floor");
	pFloor->GetTransform()->Rotate(0, 0, 90, Space::WORLD);
	Collider* pCollider = new PlaneCollider(m_pPhysics, pPhysMaterial);
	pFloor->AddComponent(pCollider);
	m_pScene->AddChild(pFloor);

	m_pDebugRenderer->SetCamera(m_pCamera->GetCamera());
}

void FluxCore::OnPause(bool isActive)
{
	if (isActive)
		GameTimer::Start();
	else
		GameTimer::Stop();
}

void FluxCore::GameLoop()
{
	m_pInput->Update();
	m_pCamera->Update();

	m_pGraphics->BeginFrame();
	m_pGraphics->Clear(ClearFlags::All, Color(0.2f, 0.2f, 0.2f, 1.0f), 1.0f, 1);

	m_pCamera->GetCamera()->SetViewport(0, 0, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight());
	
	m_pScene->Update();

	if(m_DebugPhysics)
		m_pDebugRenderer->AddPhysicsScene(m_pScene->GetComponent<PhysicsScene>());

	m_pDebugRenderer->Render();
	m_pDebugRenderer->EndFrame();

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

	ImGui::Begin("Debug");
	ImGui::Checkbox("Debug Physics", &m_DebugPhysics);
	ImGui::End();

	m_pImmediateUI->Render();
}