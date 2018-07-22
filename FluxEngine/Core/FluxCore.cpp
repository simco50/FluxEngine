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
#include "Rendering/Animation/AnimatedModel.h"
#include "Rendering/Animation/Animator.h"
#include "Rendering/Renderer.h"
#include "Rendering/Core/Texture2D.h"
#include "Rendering/Core/Texture3D.h"
#include "Content/Image.h"
#include "Rendering/Core/TextureCube.h"
#include "Rendering/ReflectionProbe.h"
#include "Scenegraph/SceneNode.h"

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
	UNREFERENCED_PARAMETER(hInstance);

	Thread::SetMainThread();
	Profiler::CreateInstance();

	AUTOPROFILE(FluxCore_Run);
	{
		AUTOPROFILE(FluxCore_Initialize);
		m_pConsole = std::make_unique<Console>();

		//Register resource locations
		FileSystem::AddPakLocation(Paths::PakFilesDir(), "Resources");
		if (!FileSystem::Mount(Paths::ResourcesDir(), "Resources", ArchiveType::Physical))
		{
			FLUX_LOG(Warning, "Failed to mount '%s'", Paths::ResourcesDir().c_str());
		}

		Config::Initialize();

		//ResourceManager
		m_pResourceManager = m_pContext->RegisterSubsystem<ResourceManager>();
		m_pResourceManager->EnableAutoReload(Config::GetBool("AutoReload", "Resources", true));

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
			FLUX_LOG(Error, "[FluxCore::Run] Failed to initialize graphics");
		}

		m_pInput = m_pContext->RegisterSubsystem<InputEngine>();
		m_pImmediateUI = m_pContext->RegisterSubsystem<ImmediateUI>();
		m_pPhysics = m_pContext->RegisterSubsystem<PhysicsSystem>();
		m_pAudioEngine = m_pContext->RegisterSubsystem<AudioEngine>();
		m_pContext->RegisterSubsystem<AsyncTaskQueue>(Misc::GetCoreCount());
		m_pDebugRenderer = m_pContext->RegisterSubsystem<DebugRenderer>();
		m_pPostProcessing = m_pContext->RegisterSubsystem<PostProcessing>();
		m_pContext->RegisterSubsystem<Renderer>();

		InitGame();
		GameTimer::Reset();
	}
	return 0;
}

void FluxCore::InitGame()
{
	AUTOPROFILE(FluxCore_InitGame);

	m_pScene = std::make_unique<Scene>(m_pContext);
	m_pCamera = m_pScene->CreateChild<FreeCamera>("Camera");
	m_pCamera->GetCamera()->SetNearPlane(10);
	m_pCamera->GetCamera()->SetFarPlane(10000);

	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Resources/Materials/LUT.xml"));
	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Resources/Materials/ChromaticAberration.xml"));

	pA = m_pScene->CreateChild("Cube A");
	Material* pDefaultMaterial = m_pResourceManager->Load<Material>("Resources/Materials/Default.xml");
	Model* pCubeModel = pA->CreateComponent<Model>();
	Mesh* pCubeMesh = m_pResourceManager->Load<Mesh>("Resources/Meshes/Cube.flux");
	std::vector<VertexElement> cubeDesc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL),
	};
	pCubeMesh->CreateBuffers(cubeDesc);
	pCubeModel->SetMesh(pCubeMesh);
	pCubeModel->SetMaterial(pDefaultMaterial);
	
	pA->GetTransform()->SetScale(20.f, 20.0f, 20.0f);

	pB = pA->CreateChild("Cube B");
	pCubeModel = pB->CreateComponent<Model>();
	pCubeModel->SetMesh(pCubeMesh);
	pCubeModel->SetMaterial(pDefaultMaterial);
	pB->GetTransform()->SetScale(12.f, 12.0f, 12.0f);
	pB->GetTransform()->Translate(16, 0.0f, 0.0f, Space::WORLD);

	pC = pB->CreateChild("Cube C");
	pCubeModel = pC->CreateComponent<Model>();
	pCubeModel->SetMesh(pCubeMesh);
	pCubeModel->SetMaterial(pDefaultMaterial);
	pC->GetTransform()->SetScale(8.f, 8.0f, 8.0f);
	pC->GetTransform()->Translate(16, 10.0f, 0.0f, Space::WORLD);
}

void FluxCore::ProcessFrame()
{
	GameTimer::Tick();
	m_pInput->Update();
	m_pConsole->FlushThreadedMessages();

	if (m_pInput->IsMouseButtonPressed(MouseKey::LEFT_BUTTON) && !ImGui::IsMouseHoveringAnyWindow())
	{
		RaycastResult result;
		if (m_pCamera->GetCamera()->Raycast(result))
		{
			m_pSelectedNode = result.pRigidbody->GetNode();
		}
		else
		{
			m_pSelectedNode = nullptr;
		}
	}
	m_pResourceManager->Update();
	m_pAudioEngine->Update();

	m_pGraphics->BeginFrame();

	pA->GetTransform()->Rotate(0, GameTimer::DeltaTime() * 50, 0);
	pB->GetTransform()->Rotate(GameTimer::DeltaTime() * 80, 0, 0, Space::SELF);
	pC->GetTransform()->Rotate(0, -GameTimer::DeltaTime() * 100, 0, Space::SELF);

	m_pScene->Update();

	m_pPostProcessing->Draw();

	if (m_DebugPhysics)
	{
		m_pDebugRenderer->AddPhysicsScene(m_pScene->GetComponent<PhysicsScene>());
	}
	if (m_pSelectedNode)
	{
		AnimatedModel* pAnimatedModel = m_pSelectedNode->GetComponent<AnimatedModel>();
		if (pAnimatedModel)
		{
			m_pDebugRenderer->AddSkeleton(pAnimatedModel->GetSkeleton(), pAnimatedModel->GetSkinMatrices(), m_pSelectedNode->GetTransform()->GetWorldMatrix(), Color(1, 0, 0, 1));
			m_pDebugRenderer->AddAxisSystem(m_pSelectedNode->GetTransform()->GetWorldMatrix(), 1.0f);
		}
		Drawable* pModel = m_pSelectedNode->GetComponent<Drawable>();
		if (pModel)
		{
			m_pDebugRenderer->AddBoundingBox(pModel->GetBoundingBox(), m_pSelectedNode->GetTransform()->GetWorldMatrix(), Color(1, 0, 0, 1), false);
		}
	}

	m_pDebugRenderer->Render();
	m_pDebugRenderer->EndFrame();

	RenderUI();
	m_pGraphics->EndFrame();
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
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Game Time : %s", time.c_str());
	ImGui::Text("MS: %f", GameTimer::DeltaTime());
	ImGui::SameLine(150);
	ImGui::Text("FPS: %f", 1.0f / GameTimer::DeltaTime());
	ImGui::Text("Primitives: %i", primitiveCount);
	ImGui::SameLine(150);
	ImGui::Text("Batches: %i", batchCount);
	ImGui::End();

	m_pInput->DrawDebugJoysticks();
	ImGui::Begin("Test");

	if (ImGui::TreeNodeEx("Debug", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Debug Physics", &m_DebugPhysics);
		ImGui::SliderFloat3("Light Position", &m_pScene->GetRenderer()->GetLightPosition()->x, -1, 1);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Resources", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Resources: %f", (float)m_pResourceManager->GetMemoryUsageOfType(Resource::GetTypeStatic()) / 1000000.0f);
		ImGui::Text("\tModels: %f", (float)m_pResourceManager->GetMemoryUsageOfType(Mesh::GetTypeStatic()) / 1000000.0f);
		ImGui::Text("\tMaterials: %f", (float)m_pResourceManager->GetMemoryUsageOfType(Material::GetTypeStatic()) / 1000000.0f);
		ImGui::Text("\tShaders: %f", (float)m_pResourceManager->GetMemoryUsageOfType(Shader::GetTypeStatic()) / 1000000.0f);
		ImGui::Text("\tImages: %f", m_pResourceManager->GetMemoryUsageOfType(Image::GetTypeStatic()) / 1000000.0f);
		ImGui::Text("\tTextures: %f", m_pResourceManager->GetMemoryUsageOfType(Texture::GetTypeStatic()) / 1000000.0f);
		ImGui::Text("\t\tTexture2D: %f", m_pResourceManager->GetMemoryUsageOfType(Texture2D::GetTypeStatic()) / 1000000.0f);
		ImGui::Text("\t\tTexture3D: %f", m_pResourceManager->GetMemoryUsageOfType(Texture3D::GetTypeStatic()) / 1000000.0f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Post Processing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (uint32 i = 0; i < m_pPostProcessing->GetMaterialCount(); ++i)
		{
			ImGui::Checkbox(m_pPostProcessing->GetMaterial(i)->GetMaterialName().c_str(), &m_pPostProcessing->GetMaterialActive(i));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Inspector", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (m_pSelectedNode)
		{
			ImGui::Text("Name: %s", m_pSelectedNode->GetName().c_str());
			ImGui::Text("Components:");
			for (const Component* pComponent : m_pSelectedNode->GetComponents())
			{
				std::string t = std::string("- ") + pComponent->GetTypeName();
				ImGui::Text(t.c_str());
			}
		}
		ImGui::TreePop();
	}
	ImGui::End();
	m_pImmediateUI->Render();
}