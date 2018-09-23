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
#include "Rendering/DebugRenderer.h"
#include "Async/AsyncTaskQueue.h"
#include "Input/InputEngine.h"
#include "Rendering/PostProcessing.h"
#include "Rendering/Animation/AnimatedModel.h"
#include "Rendering/Animation/Animator.h"
#include "Rendering/Renderer.h"
#include "Rendering/Core/Texture2D.h"
#include "Rendering/Core/Texture3D.h"
#include "Content/Image.h"
#include "Scenegraph/SceneNode.h"
#include "Rendering/Light.h"

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
	m_pContext->RegisterSubsystem<AsyncTaskQueue>(Misc::GetCoreCount() - 1);
	m_pDebugRenderer = m_pContext->RegisterSubsystem<DebugRenderer>();
	m_pPostProcessing = m_pContext->RegisterSubsystem<PostProcessing>();
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
	m_pCamera->GetTransform()->SetPosition(0, 0, -40.0f);
	m_pCamera->GetCamera()->SetNearPlane(10);
	m_pCamera->GetCamera()->SetFarPlane(10000);
	m_pDebugRenderer->SetCamera(m_pCamera->GetCamera());

	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Resources/Materials/LUT.xml"));
	/*m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Resources/Materials/ChromaticAberration.xml"));
	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Resources/Materials/Vignette.xml"));*/

	SceneNode* pPlaneNode = m_pScene->CreateChild("Floor");
	Mesh* pPlaneMesh = m_pResourceManager->Load<Mesh>("Resources/Meshes/UnitPlane.flux");
	std::vector<VertexElement> planeDesc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL),
	};
	pPlaneMesh->CreateBuffers(planeDesc);
	Model* pPlaneModel = pPlaneNode->CreateComponent<Model>();
	Material* pDefaultMaterial = m_pResourceManager->Load<Material>("Resources/Materials/Default.xml");
	pPlaneModel->SetMesh(pPlaneMesh);
	pPlaneModel->SetMaterial(pDefaultMaterial);
	pPlaneModel->GetTransform()->SetScale(5000);

	Material* pManMaterial = m_pResourceManager->Load<Material>("Resources/Materials/ManAnimated.xml");
	Mesh* pManMesh = m_pResourceManager->Load<Mesh>("Resources/Meshes/obj/Man_Walking.dae");
	std::vector<VertexElement> manDesc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::TANGENT),
		VertexElement(VertexElementType::INT4, VertexElementSemantic::BLENDINDICES),
		VertexElement(VertexElementType::FLOAT4, VertexElementSemantic::BLENDWEIGHTS),
	};
	pManMesh->CreateBuffers(manDesc);

	SceneNode* pMan = m_pScene->CreateChild("Man");
	AnimatedModel* pManModel = pMan->CreateComponent<AnimatedModel>();
	pManModel->SetMesh(pManMesh);
	pManModel->SetMaterial(pManMaterial);
	Animator* pAnimator = pMan->CreateComponent<Animator>();
	pAnimator->Play();

	SceneNode* pLight = m_pScene->CreateChild("Light");
	Light* pL = pLight->CreateComponent<Light>();
	pL->SetShadowCasting(true);
	pL->SetType(Light::Type::Directional);
	pL->SetRange(300);
	pLight->GetTransform()->Rotate(20, -50, 0);
	pLight->GetTransform()->SetPosition(0, 150, -100);
	m_Lights.push_back(pLight);
}

void FluxCore::ProcessFrame()
{
	Profiler::Instance()->Tick();
	AUTOPROFILE(FluxCore_ProcessFrame);

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

	GameUpdate();

	m_pScene->Update();
	m_pPostProcessing->Draw();

	m_pDebugRenderer->Render();
	m_pDebugRenderer->EndFrame();

	//RenderUI();
	m_pGraphics->EndFrame();
}

void FluxCore::DoExit()
{
	m_Exiting = true;
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
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
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

	m_pInput->DrawDebugJoysticks();
	ImGui::Begin("Test");

	if (ImGui::TreeNodeEx("Outliner", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int current = 0;
		const std::vector<SceneNode*>& nodes = m_pScene->GetNodes();
		ImGui::ListBox("", &current, [](void* pData, int index, const char** pLabel)
		{
			SceneNode** pNodes = (SceneNode**)pData;
			*pLabel = pNodes[index]->GetName().c_str();
			return true;
		}, (void*)nodes.data(), nodes.size());
		m_pSelectedNode = nodes[current];
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

	if (ImGui::TreeNodeEx("Debug", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Debug Rendering", &m_EnableDebugRendering);
		ImGui::Checkbox("Debug Physics", &m_DebugPhysics);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Profiling", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SliderInt("Frames", &m_FramesToCapture, 1, 10);
		if (ImGui::Button("Capture frame"))
		{
			Profiler::Instance()->Capture(m_FramesToCapture);
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Post Processing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (uint32 i = 0; i < m_pPostProcessing->GetMaterialCount(); ++i)
		{
			ImGui::Checkbox(m_pPostProcessing->GetMaterial(i)->GetName().c_str(), &m_pPostProcessing->GetMaterialActive(i));
		}
		ImGui::TreePop();
	}

	ImGui::End();

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
				m_pDebugRenderer->AddSkeleton(pAnimatedModel->GetSkeleton(), pAnimatedModel->GetSkinMatrices(), m_pSelectedNode->GetTransform()->GetWorldMatrix(), Color(1, 0, 0, 1));
				m_pDebugRenderer->AddAxisSystem(m_pSelectedNode->GetTransform()->GetWorldMatrix(), 1.0f);
			}
			Drawable* pModel = m_pSelectedNode->GetComponent<Drawable>();
			if (pModel)
			{
				m_pDebugRenderer->AddBoundingBox(pModel->GetBoundingBox(), m_pSelectedNode->GetTransform()->GetWorldMatrix(), Color(1, 0, 0, 1), false);
			}
			Light* pLight = m_pSelectedNode->GetComponent<Light>();
			if (pLight)
			{
				m_pDebugRenderer->AddLight(pLight);
			}
			m_pDebugRenderer->AddAxisSystem(m_pSelectedNode->GetTransform()->GetWorldMatrix(), 50.0f);
		}
	}
}
