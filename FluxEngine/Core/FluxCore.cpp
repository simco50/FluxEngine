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

int FluxCore::Run(HINSTANCE /*hInstance*/)
{
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

	SceneNode* pLights = m_pScene->CreateChild("Lights");

	float spacing = 350.0f;
	int countX = 4;
	int countZ = 5;

	for (int x = 0; x < countX; ++x)
	{
		for (int z = 0; z < countZ; ++z)
		{
			int idx = z + x * countZ;

			SceneNode* pLight = pLights->CreateChild(Printf("Light %d", idx));
			Light* pL = pLight->CreateComponent<Light>();
			pL->SetShadowCasting(true);
			pL->SetType(Light::Type::Point);
			pL->SetRange(300);
			pL->SetColor(Color(Math::RandomRange(0.0f, 1.0f), Math::RandomRange(0.0f, 1.0f), Math::RandomRange(0.0f, 1.0f), 1.0));
			pLight->GetTransform()->Rotate(45, 0, 0);
			pLight->GetTransform()->SetPosition(x * spacing - countX * spacing / 2.0f, 150.0f, z * spacing + 100 - countZ * spacing / 2.0f);
		}
	}
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
	m_pScene->FindNode("Lights")->GetTransform()->Rotate(0, GameTimer::DeltaTime() * 10, 0);

	m_pScene->Update();
	m_pPostProcessing->Draw();

	m_pDebugRenderer->Render();
	m_pDebugRenderer->EndFrame();

	RenderUI();
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
		name = "SceneNode";
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
		if (ImGui::Button(pNode->GetName().c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
		{
			m_pSelectedNode = pNode;
		}
		ImGui::PopID();
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

	ImGui::Begin("Outliner");
	ObjectUI(m_pScene.get());
	ImGui::End();

	m_pInput->DrawDebugJoysticks();
	ImGui::Begin("Inspector");
	if (m_pSelectedNode)
	{
		ImGui::Text("Name: %s", m_pSelectedNode->GetName().c_str());
		ImGui::Text("Components:");
		for (Component* pComponent : m_pSelectedNode->GetComponents())
		{
			std::string t = std::string("- ") + pComponent->GetTypeName();
			ImGui::Text(t.c_str());

			Light* pLight = DynamicCast<Light>(pComponent);
			if (pLight)
			{
				ImGui::Checkbox("Enabled", (bool*)&pLight->GetData()->Enabled);
				static int selected = 0;
				ImGui::Combo("Type", (int*)&pLight->GetData()->Type, [](void*, int selected, const char** pName)
				{
					Light::Type type = (Light::Type)selected;
					switch (type)
					{
					case Light::Type::Directional:
						*pName = "Directional";
						break;
					case Light::Type::Point:
						*pName = "Point";
						break;
					case Light::Type::Spot:
						*pName = "Spot";
						break;
					default:
						break;

					}
					return true;
				}, nullptr, (int)Light::Type::MAX);
				ImGui::InputFloat3("Position", &pLight->GetData()->Position.x, 1);
				pLight->GetData()->Direction.Normalize();
				ImGui::SliderFloat3("Direction", &pLight->GetData()->Direction.x, -1, 1);
				ImGui::ColorEdit4("Color", &pLight->GetData()->Colour.x);
				ImGui::SliderFloat("Intensity", &pLight->GetData()->Intensity, 0, 100);
				ImGui::SliderFloat("Range", &pLight->GetData()->Range, 0, 1000);
				ImGui::SliderFloat("SpotLightAngle", &pLight->GetData()->SpotLightAngle, 0, 180);
				ImGui::SliderFloat("Attenuation", &pLight->GetData()->Attenuation, 0, 1);
			}
		}
	}
	ImGui::End();

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Debug"))
	{
		ImGui::Checkbox("Debug Rendering", &m_EnableDebugRendering);
		ImGui::Checkbox("Debug Physics", &m_DebugPhysics);
		ImGui::Separator();
		ImGui::SliderInt("Frames", &m_FramesToCapture, 1, 10);
		if (ImGui::Button("Capture frame"))
		{
			Profiler::Instance()->Capture(m_FramesToCapture);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Post Processing"))
	{
		for (uint32 i = 0; i < m_pPostProcessing->GetMaterialCount(); ++i)
		{
			ImGui::Checkbox(m_pPostProcessing->GetMaterial(i)->GetName().c_str(), &m_pPostProcessing->GetMaterialActive(i));
		}
		ImGui::EndMenu();
	}
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
