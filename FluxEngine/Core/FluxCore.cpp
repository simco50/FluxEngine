#include "FluxEngine.h"
#include "FluxCore.h"
#include "Config.h"
#include "UI/ImmediateUI.h"

#include "Scenegraph/Scene.h"
#include "Scenegraph/Transform.h"

#include "Rendering/Core/Graphics.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Rendering/Core/Texture.h"
#include "Rendering/Mesh.h"
#include "Rendering/Model.h"
#include "Rendering/Material.h"
#include "Rendering/Camera/FreeCamera.h"
#include "Rendering/Camera/Camera.h"
#include "Physics/PhysX/PhysicsSystem.h"
#include "Physics/PhysX/PhysicsScene.h"
#include "Physics/PhysX/Rigidbody.h"
#include "Physics/PhysX/Collider.h"
#include "Window.h"
#include "Rendering/DebugRenderer.h"
#include "Context.h"
#include "Async/AsyncTaskQueue.h"

using namespace std;

FluxCore::FluxCore()
{
}

FluxCore::~FluxCore()
{
	SafeDelete(m_pContext);
	SafeDelete(m_pWindow);

	Console::Release();
	Config::Flush();
	ResourceManager::DestroyInstance();
	Profiler::DestroyInstance();
}

int FluxCore::Run(HINSTANCE hInstance)
{
	UNREFERENCED_PARAMETER(hInstance);
	Profiler::CreateInstance();

	AUTOPROFILE(FluxCore_Run);
	{
		AUTOPROFILE(FluxCore_Initialize);
		ResourceManager::CreateInstance();

		Console::Initialize();

		//Register resource locations
		FileSystem::AddPakLocation(Paths::PakFilesFolder, "Resources");
		if (!FileSystem::Mount(Paths::ResourcesFolder, "Resources", ArchiveType::Physical))
		{
			FLUX_LOG(WARNING, "Failed to mount '%s'", Paths::ResourcesFolder.c_str());
		}

		Config::Initialize();

		//Window
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

		m_pContext = new Context();
		ResourceManager::Instance()->Initialize(m_pContext);

		//Audio
		m_pContext->RegisterSubsystem<AudioEngine>();

		//Graphics
		m_pGraphics = m_pContext->RegisterSubsystem<Graphics>(m_pWindow);
		if (!m_pGraphics->SetMode(
			Config::GetBool("VSync", "Window", true),
			Config::GetInt("MSAA", "Window", 8),
			Config::GetInt("RefreshRate", "Window", 60)))
		{
			FLUX_LOG(ERROR, "[FluxCore::Run] > Failed to initialize graphics");
		}

		m_pInput = m_pContext->RegisterSubsystem<InputEngine>(m_pWindow);
		m_pImmediateUI = m_pContext->RegisterSubsystem<ImmediateUI>(m_pWindow);
		m_pPhysics = m_pContext->RegisterSubsystem<PhysicsSystem>(nullptr);
		m_pContext->RegisterSubsystem<AsyncTaskQueue>(4);

		m_pDebugRenderer = m_pContext->RegisterSubsystem<DebugRenderer>();

		InitGame();
		GameTimer::Reset();
	}

	//Message loop
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

	m_pScene = make_unique<Scene>(m_pContext);
	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, 1, 1), true);
	m_pCamera = new FreeCamera(m_pContext);
	m_pScene->AddChild(m_pCamera);

	PxMaterial* pPhysMaterial = m_pPhysics->GetPhysics()->createMaterial(0.6f, 0.6f, 0.1f);

	Mesh* pMesh = ResourceManager::Instance()->Load<Mesh>("Resources/Meshes/Spot.flux");
	std::vector<VertexElement> desc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL)
	};
	pMesh->CreateBuffers(desc);

	Material* pMaterial = ResourceManager::Instance()->Load<Material>("Resources/Materials/Default.xml");

	for (int i = 0; i < 10; ++i)
	{
		m_pNode = new SceneNode(m_pContext, "Cube");
		m_pNode->GetTransform()->Translate(0, 2.0f * i + 2.0f, 0);
		Model* pModel = new Model(m_pContext);
		pModel->SetMesh(pMesh);
		pModel->SetMaterial(pMaterial);
		m_pNode->AddComponent(pModel);
		Rigidbody* pRigidbody = new Rigidbody(m_pContext);
		pRigidbody->SetBodyType(Rigidbody::Dynamic);
		m_pNode->AddComponent(pRigidbody);
		Collider* pBoxCollider = new MeshCollider(m_pContext, "Resources/Meshes/Spot.collision");
		m_pNode->AddComponent(pBoxCollider);
		m_pScene->AddChild(m_pNode);
	}
	SceneNode* pFloor = new SceneNode(m_pContext, "Floor");
	pFloor->GetTransform()->Rotate(0, 0, 90, Space::WORLD);
	Collider* pPlaneCollider = new PlaneCollider(m_pContext, pPhysMaterial);
	pFloor->AddComponent(pPlaneCollider);
	m_pScene->AddChild(pFloor);

	m_pDebugRenderer->SetCamera(m_pCamera->GetCamera());

	m_pInput->AddInputAction(InputAction(0, Pressed, -1, VK_LBUTTON));
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
	m_pCamera->GetCamera()->SetViewport(0, 0, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight());

	m_pGraphics->BeginFrame();
	m_pGraphics->Clear(ClearFlags::All, Color(0.2f, 0.2f, 0.2f, 1.0f), 1.0f, 1);
	
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