#include "FluxEngine.h"
#include "FluxCore.h"
#include "Config.h"
#include "UI/ImmediateUI.h"

#include "Scenegraph/Scene.h"
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
#include "Rendering/DebugRenderer.h"
#include "Async/AsyncTaskQueue.h"
#include "Input/InputEngine.h"
#include "Rendering/PostProcessing.h"
#include "Rendering/Animation/AnimatedModel.h"
#include "Rendering/Animation/Animator.h"
#include "Rendering/Renderer.h"
#include "Scenegraph/SceneNode.h"
#include "Rendering/Light.h"
#include "Rendering/ReflectionProbe.h"
#include "Rendering/ParticleSystem/ParticleEmitter.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"

#include "Math/DualQuaternion.h"
#include "CommandLine.h"
#include "Physics/PhysX/Collider.h"
#include "Rendering/Animation/Animation.h"
#include "Rendering/Core/Shader.h"
#include "Rendering/Geometry.h"

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

int FluxCore::Run(HINSTANCE /*hInstance*/)
{
	Thread::SetMainThread();
	Profiler::CreateInstance();

	AUTOPROFILE(FluxCore_Initialize);
	m_pConsole = std::make_unique<Console>();

	//Register resource locations
	if (CommandLine::GetBool("NoPak") == false)
	{
		FileSystem::AddPakLocation(Paths::PakFilesDir());
	}
	if (!FileSystem::Mount(Paths::ResourcesDir()))
	{
		FLUX_LOG(Warning, "Failed to mount '%s'", Paths::ResourcesDir().c_str());
	}

	Config::Initialize();

	//ResourceManager
	m_pResourceManager = m_pContext->RegisterSubsystem<ResourceManager>();
	m_pResourceManager->EnableAutoReload(Config::GetBool("AutoReload", "Resources", true));

	//Graphics

	GraphicsCreateInfo createInfo = {};
	m_pGraphics = m_pContext->RegisterSubsystem<Graphics>();
	if (!m_pGraphics->SetMode(createInfo))
	{
		FLUX_LOG(Error, "[FluxCore::Run] Failed to initialize graphics");
	}

	m_pInput = m_pContext->RegisterSubsystem<InputEngine>();
	//m_pImmediateUI = m_pContext->RegisterSubsystem<ImmediateUI>();
	m_pPhysics = m_pContext->RegisterSubsystem<PhysicsSystem>();
	m_pAudioEngine = m_pContext->RegisterSubsystem<AudioEngine>();
	AsyncTaskQueue* pQueue = m_pContext->RegisterSubsystem<AsyncTaskQueue>();
	pQueue->Initialize(Misc::GetCoreCount() - 1);
	m_pDebugRenderer = m_pContext->RegisterSubsystem<DebugRenderer>();
	m_pContext->RegisterSubsystem<Renderer>();

	InitGame();
	GameTimer::Reset();
	return 0;
}

void FluxCore::InitGame()
{
	AUTOPROFILE(FluxCore_InitGame);

	m_pScene = std::make_unique<Scene>(m_pContext);
	m_pCamera = m_pScene->CreateChild<FreeCamera>("Camera");
	m_pCamera->SetPosition(0, 0, -40.0f);
	Camera* pCamera = m_pCamera->GetCamera();
	pCamera->SetNearPlane(10);
	pCamera->SetFarPlane(10000);
	m_pDebugRenderer->SetCamera(&m_pCamera->GetCamera()->GetViewData());
}

void FluxCore::ProcessFrame()
{
	Profiler::Instance()->Tick();
	AUTOPROFILE(FluxCore_ProcessFrame);

	GameTimer::Tick();
	m_pInput->Update();
	m_pConsole->FlushThreadedMessages();

	//if (m_pInput->IsMouseButtonPressed(MouseKey::LEFT_BUTTON) && !ImGui::GetIO().WantCaptureMouse)
	//{
	//	Vector3 position, direction;
	//	Ray ray = m_pCamera->GetCamera()->GetMouseRay();
	//	m_pSelectedNode = m_pScene->PickNode(ray);
	//}
	m_pResourceManager->Update();
	m_pAudioEngine->Update();
	m_pGraphics->BeginFrame();

	//GameUpdate();
	//m_pScene->FindNode("MainCube")->Rotate(0, GameTimer::DeltaTime() * 50, 0, Space::Self);
	//m_pScene->FindNode("SecondCube")->Rotate(0, 0, GameTimer::DeltaTime() * 100, Space::Self);
	//m_pScene->FindNode("LastCube")->Rotate(-GameTimer::DeltaTime() * 80, 0, 0, Space::Self);

	//m_pScene->Update();

	//m_pDebugRenderer->Render();
	//m_pDebugRenderer->EndFrame();

	//RenderUI();
	m_pGraphics->EndFrame();
}

void FluxCore::DoExit()
{
	m_Exiting = true;
}

void FluxCore::ObjectUI(SceneNode* pNode)
{
	if (pNode == nullptr)
	{
		return;
	}
	const std::vector<SceneNode*> pChildren = pNode->GetChildren();

	std::string name = pNode->GetName();
	if (name.empty())
	{
		name = "SceneNode";
	}
	if (pChildren.empty() == false)
	{
		ImGui::PushID(pNode);
		if (ImGui::Button("   "))
		{
			m_pSelectedNode = pNode;
		}
		ImGui::PopID();
		ImGui::SameLine();
		if (ImGui::TreeNode(name.c_str()))
		{
			for (SceneNode* pChild : pChildren)
			{
				ObjectUI(pChild);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::PushID(pNode);
		if (ImGui::Button("   "))
		{
			m_pSelectedNode = pNode;
		}
		ImGui::SameLine();
		ImGui::Text("%s", pNode->GetName().c_str());
		ImGui::PopID();
	}
}

void FluxCore::ComponentUI(StringHash type)
{
	std::vector<const TypeInfo*> types = m_pContext->GetAllTypesOf(type, false, true);
	for (const TypeInfo* pType : types)
	{
		if (pType->IsAbstract())
		{
			if (ImGui::BeginMenu(pType->GetTypeName()))
			{
				ComponentUI(pType->GetType());
				ImGui::EndMenu();
			}
		}
		else if (ImGui::MenuItem(pType->GetTypeName()))
		{
			if (pType->IsAbstract() == false && m_pSelectedNode)
			{
				m_pSelectedNode->CreateComponent(pType->GetType());
			}
		}
	}
}

void FluxCore::RenderUI()
{
	AUTOPROFILE(FluxCore_RenderUI);
	m_pImmediateUI->NewFrame();

	unsigned int batchCount, primitiveCount;
	m_pGraphics->GetDebugInfo(batchCount, primitiveCount);

	std::stringstream timeStr;
	timeStr << std::setw(2) << std::setfill('0') << (int)GameTimer::GameTime() / 60 << ":" << std::setw(2) << (int)GameTimer::GameTime() % 60;
	std::string time = timeStr.str();
	ImGui::SetNextWindowPos(ImVec2((float)m_pGraphics->GetWindowWidth(), 30), 0, ImVec2(1, 0));
	ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Game Time : %s", time.c_str());
	ImGui::Text("Configuration: %s", BuildConfiguration::ToString(BuildConfiguration::Configuration));
	ImGui::SameLine(150);
	ImGui::Text("Platform: %s", BuildPlatform::ToString(BuildPlatform::Platform));
	ImGui::Text("MS: %f", GameTimer::DeltaTime());
	ImGui::SameLine(150);
	ImGui::Text("FPS: %f", 1.0f / GameTimer::DeltaTime());
	ImGui::Text("Primitives: %i", primitiveCount);
	ImGui::SameLine(150);
	ImGui::Text("Batches: %i", batchCount);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 20.0f));
	ImGui::SetNextWindowSize(ImVec2(300.0f, m_pGraphics->GetWindowHeight() - 20.0f));
	ImGui::Begin("Outliner", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("Outline", ImVec2(300, ImGui::GetContentRegionAvail().y / 2), true, ImGuiWindowFlags_AlwaysAutoResize);
	ObjectUI(m_pScene.get());
	ImGui::EndChild();
	ImGui::Separator();
	ImGui::BeginChild("Inspector", ImVec2(300, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_AlwaysAutoResize);
	if (m_pSelectedNode)
	{
		ImGui::Text("Name: %s", m_pSelectedNode->GetName().c_str());
		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::InputFloat3("Position", const_cast<float*>(&m_pSelectedNode->GetPosition().x)))
			{
				m_pSelectedNode->MarkDirty();
			}
			if (ImGui::InputFloat3("Scale", const_cast<float*>(&m_pSelectedNode->GetScale().x)))
			{
				m_pSelectedNode->MarkDirty();
			}
			ImGui::InputFloat4("Rotation", const_cast<float*>(&m_pSelectedNode->GetRotation().x), "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::TreePop();
		}

		for (Component* pComponent : m_pSelectedNode->GetComponents())
		{
			if (ImGui::TreeNode(pComponent->GetTypeName()))
			{
				pComponent->CreateUI();
				ImGui::TreePop();
			}
		}
	}
	ImGui::EndChild();
	ImGui::End();

	m_pInput->DrawDebugJoysticks();

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Debug"))
	{
		ImGui::Checkbox("Debug Rendering", &m_EnableDebugRendering);
		ImGui::Checkbox("Debug Physics", &m_DebugPhysics);
		ImGui::Separator();
		ImGui::SliderInt("Frames", &m_FramesToCapture, 1, 10);
		if (ImGui::MenuItem("Capture frame"))
		{
			Profiler::Instance()->Capture(m_FramesToCapture);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Selected Node"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_pSelectedNode ? 1.0f : 0.5f);
		if (ImGui::MenuItem("Create Child"))
		{
			if (m_pSelectedNode)
			{
				m_pSelectedNode = m_pSelectedNode->CreateChild("Node");
			}
		}
		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_pSelectedNode ? 1.0f : 0.5f);
	if (ImGui::BeginMenu("Create Component"))
	{
		if (m_pSelectedNode)
		{
			ComponentUI(Component::GetTypeStatic());
		}
		ImGui::EndMenu();
	}
	ImGui::PopStyleVar();
	ImGui::EndMainMenuBar();

	m_pImmediateUI->Render();
}

void FluxCore::GameUpdate()
{
	AUTOPROFILE(FluxCore_GameUpdate);

	if (m_EnableDebugRendering)
	{
		if (m_DebugPhysics)
		{
			m_pDebugRenderer->AddPhysicsScene(m_pScene->GetComponent<PhysicsScene>());
		}
		if (m_pSelectedNode)
		{
			AnimatedModel* pAnimatedModel = m_pSelectedNode->GetComponent<AnimatedModel>();
			if (pAnimatedModel)
			{
				m_pDebugRenderer->AddSkeleton(pAnimatedModel->GetSkeleton(), Color(1, 1, 0, 1));
				m_pDebugRenderer->AddAxisSystem(m_pSelectedNode->GetWorldMatrix());
			}
			Drawable* pModel = m_pSelectedNode->GetComponent<Drawable>();
			if (pModel)
			{
				m_pDebugRenderer->AddBoundingBox(pModel->GetBoundingBox(), m_pSelectedNode->GetWorldMatrix(), Color(1, 0, 0, 1), false);
			}
			Light* pLight = m_pSelectedNode->GetComponent<Light>();
			if (pLight)
			{
				m_pDebugRenderer->AddLight(pLight);
			}
			m_pDebugRenderer->AddAxisSystem(m_pSelectedNode->GetWorldMatrix());
		}
	}
}
