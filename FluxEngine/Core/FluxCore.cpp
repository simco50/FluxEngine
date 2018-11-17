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
	m_pCamera->SetPosition(0, 0, -40.0f);
	m_pCamera->GetCamera()->SetNearPlane(10);
	m_pCamera->GetCamera()->SetFarPlane(10000);
	m_pDebugRenderer->SetCamera(m_pCamera->GetCamera());

	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Materials/LUT.xml"));
	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Materials/Vignette.xml"));
	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Materials/ChromaticAberration.xml"));
	m_pPostProcessing->AddEffect(m_pResourceManager->Load<Material>("Materials/FXAA.xml"));

	SceneNode* pPlaneNode = m_pScene->CreateChild("Floor");
	Mesh* pPlaneMesh = m_pResourceManager->Load<Mesh>("Meshes/UnitPlane.flux");
	std::vector<VertexElement> planeDesc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL),
	};
	pPlaneMesh->CreateBuffers(planeDesc);
	Model* pPlaneModel = pPlaneNode->CreateComponent<Model>();
	Material* pDefaultMaterial = m_pResourceManager->Load<Material>("Materials/Default.xml");
	pPlaneModel->SetMesh(pPlaneMesh);
	pPlaneModel->SetMaterial(pDefaultMaterial);
	pPlaneNode->SetScale(5000);
	pPlaneNode->CreateComponent<Rigidbody>();
	pPlaneNode->CreateComponent<PlaneCollider>();

	Mesh* pManMesh = m_pResourceManager->Load<Mesh>("Meshes/obj/Man_Walking.dae");
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

	{
		Material* pManMaterial = m_pResourceManager->Load<Material>("Materials/ManAnimated_DualQuaternion.xml");
		SceneNode* pMan = m_pScene->CreateChild("Man - Dual Quaternion Skinning");
		AnimatedModel* pManModel = pMan->CreateComponent<AnimatedModel>();
		pManModel->SetMesh(pManMesh);
		pManModel->SetMaterial(pManMaterial);
		Animator* pAnimator = pMan->CreateComponent<Animator>();
		pAnimator->Play();
	}

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
			pLight->Rotate(45, 0, 0);
			pLight->SetPosition(x * spacing - countX * spacing / 2.0f, 150.0f, z * spacing + 100 - countZ * spacing / 2.0f);
		}
	}

	/*SceneNode* pLight = m_pScene->CreateChild("Light");
	pLight->CreateComponent<Light>();
	pLight->Rotate(45, 0, 0);

	Mesh* pCubeMesh = m_pResourceManager->Load<Mesh>("Meshes/Cube.flux");
	std::vector<VertexElement> cubeDesc =
	{
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
		VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
		VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL),
	};
	pCubeMesh->CreateBuffers(cubeDesc);
	Material* pDefaultMaterial = m_pResourceManager->Load<Material>("Materials/Default.xml");

	SceneNode* pMainCube = m_pScene->CreateChild("MainCube");
	pMainCube->SetScale(5);
	SceneNode* pSecondCube = pMainCube->CreateChild("SecondCube");
	pSecondCube->SetScale(3);
	pSecondCube->Translate(0, 0, 4);
	SceneNode* pLastCube = pSecondCube->CreateChild("LastCube");
	pLastCube->SetScale(2);
	pLastCube->Translate(2.5f, 0, 4);

	Model* pModel = pMainCube->CreateComponent<Model>();
	pModel->SetMesh(pCubeMesh);
	pModel->SetMaterial(pDefaultMaterial);

	pModel = pSecondCube->CreateComponent<Model>();
	pModel->SetMesh(pCubeMesh);
	pModel->SetMaterial(pDefaultMaterial);

	pModel = pLastCube->CreateComponent<Model>();
	pModel->SetMesh(pCubeMesh);
	pModel->SetMaterial(pDefaultMaterial);*/
}

void FluxCore::ProcessFrame()
{
	Profiler::Instance()->Tick();
	AUTOPROFILE(FluxCore_ProcessFrame);

	GameTimer::Tick();
	m_pInput->Update();
	m_pConsole->FlushThreadedMessages();

	if (m_pInput->IsMouseButtonPressed(MouseKey::LEFT_BUTTON) && !ImGui::GetIO().WantCaptureMouse)
	{
		Vector3 position, direction;
		Ray ray = m_pCamera->GetCamera()->GetMouseRay();
		m_pSelectedNode = m_pScene->PickNode(ray);
	}
	m_pResourceManager->Update();
	m_pAudioEngine->Update();
	m_pGraphics->BeginFrame();

	GameUpdate();
	//m_pScene->FindNode("MainCube")->Rotate(0, GameTimer::DeltaTime() * 50, 0, Space::Self);
	//m_pScene->FindNode("SecondCube")->Rotate(0, 0, GameTimer::DeltaTime() * 100, Space::Self);
	//m_pScene->FindNode("LastCube")->Rotate(-GameTimer::DeltaTime() * 80, 0, 0, Space::Self);

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
		ImGui::Text(pNode->GetName().c_str());
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
			ImGui::TreePop();
		}

		for (Component* pComponent : m_pSelectedNode->GetComponents())
		{
			if (ImGui::TreeNode(pComponent->GetTypeName().c_str()))
			{
				pComponent->CreateUI();
				ImGui::TreePop();
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
				m_pDebugRenderer->AddSkeleton(pAnimatedModel->GetSkeleton(), pAnimatedModel->GetSkinMatrices(), m_pSelectedNode->GetWorldMatrix(), Color(1, 0, 0, 1));
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
