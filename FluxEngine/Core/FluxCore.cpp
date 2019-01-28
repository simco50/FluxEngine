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

	PostProcessing* post = m_pCamera->CreateComponent<PostProcessing>();
	post->AddEffect(m_pResourceManager->Load<Material>("Materials/LUT.xml"));
	post->AddEffect(m_pResourceManager->Load<Material>("Materials/Vignette.xml"));
	post->AddEffect(m_pResourceManager->Load<Material>("Materials/ChromaticAberration.xml"));
	post->AddEffect(m_pResourceManager->Load<Material>("Materials/FXAA.xml"));

	/*SceneNode* pPlaneNode = m_pScene->CreateChild("Floor");
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

	Material* pManMaterial = m_pResourceManager->Load<Material>("Materials/ManAnimated.xml");
	SceneNode* pMan = m_pScene->CreateChild("Man - Matrix Skinning - DAE");
	AnimatedModel* pManModel = pMan->CreateComponent<AnimatedModel>();
	pManModel->SetMesh(pManMesh);
	pManModel->SetMaterial(pManMaterial);
	Animator* pAnimator = pMan->CreateComponent<Animator>();
	Animation* pAnimation = m_pResourceManager->Load<Animation>("Meshes/obj/Man_Walking.dae");
	pAnimator->Play(pAnimation);

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
	}*/

	{
		SceneNode* pLight = m_pScene->CreateChild("Light 0");
		Light* pL = pLight->CreateComponent<Light>();
		pL->SetShadowCasting(true);
		pL->SetType(Light::Type::Directional);
		pL->SetRange(300);
		pL->SetColor(Color(1,1,1,1));
		pLight->Rotate(45, -135, 0);
	}

	{
		Material* pMat = m_pResourceManager->Load<Material>("Materials/TessellationExample.xml");
		Mesh* pCubeMesh = m_pResourceManager->Load<Mesh>("Meshes/obj/plane.obj");
		std::vector<VertexElement> cubeDesc =
		{
			VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
			VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
			VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL),
		};
		pCubeMesh->CreateBuffers(cubeDesc);
		pCubeMesh->GetGeometry(0)->SetDrawRange(PrimitiveType::PATCH_CP_3, pCubeMesh->GetGeometry(0)->GetIndexCount(), pCubeMesh->GetGeometry(0)->GetVertexCount());
		SceneNode* pCubeNode = m_pScene->CreateChild("Tessellated thing");
		Model* pCubeModel = pCubeNode->CreateComponent<Model>();
		pCubeModel->SetMesh(pCubeMesh);
		pCubeModel->SetMaterial(pMat);
		pCubeNode->SetScale(50);
	}


	/*{
		Mesh* pMesh = m_pResourceManager->Load<Mesh>("Meshes/obj/Pot.dae");
		std::vector<VertexElement> meshDesc =
		{
			VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::POSITION),
			VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD),
			VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD, 1),
			VertexElement(VertexElementType::FLOAT3, VertexElementSemantic::NORMAL),
		};
		pMesh->CreateBuffers(meshDesc);
		Material* pMaterial = m_pResourceManager->Load<Material>("Materials/VertexAnimation.xml");
		SceneNode* pNode = m_pScene->CreateChild("Vertex Animation");
		Model* pModel = pNode->CreateComponent<Model>();
		pModel->SetMesh(pMesh);
		pModel->SetMaterial(pMaterial);
	}*/
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
		if (ImGui::MenuItem("Create Child"))
		{
			if (m_pSelectedNode)
			{
				m_pSelectedNode = m_pSelectedNode->CreateChild("Node");
			}
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Create Component"))
	{
		ComponentUI(Component::GetTypeStatic());
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
