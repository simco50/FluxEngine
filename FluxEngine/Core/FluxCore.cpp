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
#include "Rendering/Material.h"
#include "Rendering/Camera/FreeCamera.h"
#include "Rendering/Camera/Camera.h"
#include "Physics/PhysX/PhysicsSystem.h"
#include "Physics/PhysX/PhysicsScene.h"
#include "Physics/PhysX/Rigidbody.h"
#include "Physics/PhysX/Collider.h"
#include "Rendering/DebugRenderer.h"
#include "Context.h"
#include "Async/AsyncTaskQueue.h"
#include "Async/Thread.h"
#include "Input/InputEngine.h"
#include "Rendering/Core/Shader.h"
#include "Rendering/PostProcessing.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/ParticleSystem/ParticleEmitter.h"
#include "Rendering/AnimatedModel.h"

bool FluxCore::m_Exiting;

FluxCore::FluxCore(Context* pContext) :
	Object(pContext)
{
}

FluxCore::~FluxCore()
{
	Config::Flush();
	Profiler::DestroyInstance();
}

int FluxCore::Run(HINSTANCE hInstance)
{
	Thread::SetMainThread();
	UNREFERENCED_PARAMETER(hInstance);
	Profiler::CreateInstance();

	AUTOPROFILE(FluxCore_Run);
	{
		AUTOPROFILE(FluxCore_Initialize);
		m_pConsole = std::make_unique<Console>();

		//Register resource locations
		FileSystem::AddPakLocation(Paths::PakFilesFolder, "Resources");
		if (!FileSystem::Mount(Paths::ResourcesFolder, "Resources", ArchiveType::Physical))
		{
			FLUX_LOG(Warning, "Failed to mount '%s'", Paths::ResourcesFolder.c_str());
		}

		Config::Initialize();

		//ResourceManager
		m_pResourceManager = m_pContext->RegisterSubsystem<ResourceManager>();
		m_pResourceManager->EnableAutoReload();

		//Graphics
		m_pGraphics = m_pContext->RegisterSubsystem<Graphics>();
		if (!m_pGraphics->SetMode(
			Config::GetString("Title", "Window", "FluxEngine"),
			Config::GetInt("Width", "Window", 1240),
			Config::GetInt("Height", "Window", 720),
			(WindowType)Config::GetInt("WindowMode", "Window", 0),
			Config::GetBool("Resizable", "Window", true),
			Config::GetBool("VSync", "Window", true),
			Config::GetInt("MSAA", "Window", 1),
			Config::GetInt("RefreshRate", "Window", 60)))
		{
			FLUX_LOG(Error, "[FluxCore::Run] > Failed to initialize graphics");
		}
		m_pInput = m_pContext->RegisterSubsystem<InputEngine>();
		m_pImmediateUI = m_pContext->RegisterSubsystem<ImmediateUI>();
		m_pPhysics = m_pContext->RegisterSubsystem<PhysicsSystem>(nullptr);
		m_pAudioEngine = m_pContext->RegisterSubsystem<AudioEngine>();
		m_pContext->RegisterSubsystem<AsyncTaskQueue>(4);
		m_pDebugRenderer = m_pContext->RegisterSubsystem<DebugRenderer>();
		m_pPostProcessing = m_pContext->RegisterSubsystem<PostProcessing>();

		InitGame();
		GameTimer::Reset();
	}
	return 0;
}

void FluxCore::InitGame()
{
	AUTOPROFILE(FluxCore_InitGame);

	m_pScene = std::make_unique<Scene>(m_pContext);
	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, 1, 1), true);
	m_pCamera = new FreeCamera(m_pContext);
	m_pScene->AddChild(m_pCamera);
	m_pCamera->GetCamera()->SetFarPlane(1000);
	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Resources/Materials/LUT.xml"));

	Mesh* pMesh = m_pResourceManager->Load<Mesh>("Resources/Meshes/obj/Man_Drunk.dae");
	std::vector<VertexElement> desc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL),
		VertexElement(VertexElementType::INT4, VertexElementSemantic::BLENDINDICES),
		VertexElement(VertexElementType::FLOAT4, VertexElementSemantic::BLENDWEIGHTS),
	};
	pMesh->CreateBuffers(desc);

	Material* pMaterial = m_pResourceManager->Load<Material>("Resources/Materials/DefaultSkinned.xml");

	SceneNode* pObject = new SceneNode(m_pContext, "Cube");
	AnimatedModel* pModel = new AnimatedModel(m_pContext);
	pModel->SetMesh(pMesh);
	pModel->SetMaterial(pMaterial);
	pObject->AddComponent(pModel);
	m_pScene->AddChild(pObject);

	Rigidbody* pRigidbody = new Rigidbody(m_pContext);
	BoxCollider* pCollider = new BoxCollider(m_pContext, pModel->GetBoundingBox());
	pObject->AddComponent(pRigidbody);
	pObject->AddComponent(pCollider);

	m_pDebugRenderer->SetCamera(m_pCamera->GetCamera());

	m_pInput->AddInputAction(InputAction(0, Pressed, -1, VK_LBUTTON));
}

void FluxCore::ProcessFrame()
{
	GameTimer::Tick();
	m_pInput->Update();
	m_pConsole->FlushThreadedMessages();

	m_pCamera->GetCamera()->SetViewport(0, 0, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight());

	if (m_pInput->IsMouseButtonPressed(MouseKey::LEFT_BUTTON) && !ImGui::IsMouseHoveringAnyWindow())
	{
		RaycastResult result;
		if (m_pCamera->GetCamera()->Raycast(result))
		{
			m_pSelectedNode = result.pRigidbody->GetNode();
		}
		else
			m_pSelectedNode = nullptr;
	}

	m_pGraphics->BeginFrame();
	m_pGraphics->Clear(ClearFlags::All, Color(0.2f, 0.2f, 0.2f, 1.0f), 1.0f, 1);

	m_pAudioEngine->Update();
	m_pScene->Update();

	m_pPostProcessing->Draw();

	if (m_DebugPhysics)
		m_pDebugRenderer->AddPhysicsScene(m_pScene->GetComponent<PhysicsScene>());

	if (m_pSelectedNode)
	{
		AnimatedModel* pModel = m_pSelectedNode->GetComponent<AnimatedModel>();
		m_pDebugRenderer->AddSkeleton(pModel->GetSkeleton(), pModel->GetSkinMatrices(), m_pSelectedNode->GetTransform()->GetWorldMatrix(), Color(1, 0, 0, 1));
	}

	m_pDebugRenderer->Render();
	m_pDebugRenderer->EndFrame();

	RenderUI();
	m_pGraphics->EndFrame();
	m_pResourceManager->Update();
}

void FluxCore::DoExit()
{
	m_Exiting = true;
}

void FluxCore::RenderUI()
{
	m_pImmediateUI->NewFrame();

	unsigned int batchCount, primitiveCount;
	m_pGraphics->GetDebugInfo(batchCount, primitiveCount);

	std::stringstream timeStr;
	timeStr << std::setw(2) << std::setfill('0') << (int)GameTimer::GameTime() / 60 << ":" << std::setw(2) << (int)GameTimer::GameTime() % 60;
	std::string time = timeStr.str();
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Game Time : %s", time.c_str());
	ImGui::Text("MS: %f", GameTimer::DeltaTime());
	ImGui::SameLine(150);
	ImGui::Text("FPS: %f", 1.0f / GameTimer::DeltaTime());
	ImGui::Text("Primitives: %i", primitiveCount);
	ImGui::SameLine(150);
	ImGui::Text("Batches: %i", batchCount);
	ImGui::End();

	if (ImGui::TreeNodeEx("Debug", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Debug Physics", &m_DebugPhysics);
		ImGui::TreePop();
	}
	if (ImGui::TreeNodeEx("Resources", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Button("Reload shaders", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
		{
			for (Resource* pResource : m_pResourceManager->GetResourcesOfType(Shader::GetTypeStatic()))
				m_pResourceManager->Reload(pResource);
		}
		if (ImGui::Button("Reload materials", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
		{
			for (Resource* pResource : m_pResourceManager->GetResourcesOfType(Material::GetTypeStatic()))
				m_pResourceManager->Reload(pResource);
		}
		if (ImGui::Button("Reload textures", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
		{
			for (Resource* pResource : m_pResourceManager->GetResourcesOfType(Texture::GetTypeStatic()))
				m_pResourceManager->Reload(pResource);
		}
		ImGui::TreePop();
	}
	if (m_pSelectedNode && ImGui::TreeNodeEx("Inspector", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text(m_pSelectedNode->GetName().c_str());
		ImGui::Text("Components:");
		for (const Component* pComponent : m_pSelectedNode->GetComponents())
		{
			std::string t = std::string("- ") + pComponent->GetTypeName();
			ImGui::Text(t.c_str());
		}
		m_pDebugRenderer->AddAxisSystem(m_pSelectedNode->GetTransform()->GetWorldMatrix(), 1.0f);
		Drawable* pModel = m_pSelectedNode->GetComponent<Drawable>();
		if (pModel)
		{
			m_pDebugRenderer->AddBoundingBox(pModel->GetBoundingBox(), m_pSelectedNode->GetTransform()->GetWorldMatrix(), Color(1, 0, 0, 1), false);
		}
		ImGui::TreePop();
	}


	m_pImmediateUI->Render();
}