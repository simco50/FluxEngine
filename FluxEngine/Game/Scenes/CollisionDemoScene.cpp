#include "stdafx.h"
#include "CollisionDemoScene.h"
#include "../../Scenegraph/GameObject.h"
#include "../../Components/MeshComponent.h"
#include "../../Helpers/MathHelp.h"
#include "../../Materials/Forward/DefaultMaterial.h"
#include "../../Components/TransformComponent.h"
#include <ctime>
#include "../../Game/GameManager.h"
#include "../../Physics/Flex/FlexSoftbody.h"
#include "../../Managers/SoundManager.h"
#include "../../Physics/Flex/FlexMousePicker.h"
#include "../../Physics/Flex/FlexDebugRenderer.h"
#include "../../Materials/Deferred/BasicMaterial_Deferred.h"
#include "../../Physics/Flex/FlexSystem.h"
#include "../../Physics/Flex/FlexRigidbody.h"
#include "../../UI/ImgUI/imgui.h"
#include "../../Prefabs/Skybox.h"
#include "../../Physics/Flex/FlexTriangleMeshCollider.h"
#include "Graphics/Texture.h"

CollisionDemoScene::CollisionDemoScene()
{}

CollisionDemoScene::~CollisionDemoScene()
{
	SafeDelete(m_pFlexMousePicker);
	SafeDelete(m_pFlexSystem);
}

void CollisionDemoScene::Initialize()
{
	m_pFlexSystem = new FlexSystem();

	Skybox* pSkybox = new Skybox();
	AddChild(pSkybox);
	pSkybox->SetTexture(ResourceManager::Load<Texture>(L"./Resources/GradWork/Skyboxes/skybox_gray.dds"));

	//Create the ground plane
	GameObject* pGroundPlane = new GameObject();
	MeshComponent* pMeshComponent = new MeshComponent(L"./Resources/Meshes/unit_plane.flux");

	m_pGroundMaterial = make_unique<DefaultMaterial>();
	m_pGroundMaterial->Initialize(m_pGameContext);
	m_pGroundMaterial->SetDiffuseTexture(L"./Resources/GradWork/Textures/gradientground.png");
	pMeshComponent->SetMaterial(m_pGroundMaterial.get());
	pGroundPlane->AddComponent(pMeshComponent);
	pGroundPlane->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	pGroundPlane->GetTransform()->SetScale(15.0f, 15.0f, 15.0f);
	AddChild(pGroundPlane);

	m_pDefaultMaterial = make_unique<DefaultMaterial>();
	m_pDefaultMaterial->Initialize(m_pGameContext);

	//Create the collision meshes
	m_pSpinnerObject = new GameObject();
	MeshComponent* pMesh = new MeshComponent(L"./Resources/GradWork/Meshes/collision_spinner.flux");
	pMesh->SetMaterial(m_pDefaultMaterial.get());
	FlexTriangleMeshCollider* pCollider = new FlexTriangleMeshCollider(m_pFlexSystem);
	m_pSpinnerObject->AddComponent(pMesh);
	m_pSpinnerObject->AddComponent(pCollider);
	AddChild(m_pSpinnerObject);
	m_pSpinnerObject->GetTransform()->SetPosition(0.0f, 5.0f, 0.0f);
	m_pSpinnerObject->GetTransform()->SetScale(2.0f, 2.0f, 2.0f);

	m_pContainerObject = new GameObject();
	pMesh = new MeshComponent(L"./Resources/GradWork/Meshes/collision_container.flux");
	pMesh->SetMaterial(m_pDefaultMaterial.get());
	pCollider = new FlexTriangleMeshCollider(m_pFlexSystem);
	m_pContainerObject->AddComponent(pMesh);
	m_pContainerObject->AddComponent(pCollider);
	AddChild(m_pContainerObject);
	m_pContainerObject->GetTransform()->SetPosition(0.0f, 5.0f, 0.0f);
	m_pContainerObject->GetTransform()->SetScale(2.0f, 2.0f, 2.0f);

	//Set default flex params
	m_pFlexSystem->SetDefaultParams();
	m_pFlexSystem->CreateGroundPlane();

	m_pFlexSystem->Params.mRadius = 0.07f;
	m_pFlexSystem->Params.mDynamicFriction = 0.35f;
	m_pFlexSystem->Params.mNumIterations = 4;

	//Create the softbodies
	SoftbodyDesc sDesc;
	sDesc.Radius = m_pFlexSystem->Params.mRadius;
	sDesc.VolumeSampling = 4.0f;
	sDesc.SurfaceSampling = 1.5f;
	sDesc.ClusterSpacing = 2.5f;
	sDesc.ClusterRadius = 2.7f;
	sDesc.ClusterStiffness = 0.1f;
	sDesc.LinkRadius = 0.0f;
	sDesc.LinkStiffness = 1.0f;
	sDesc.Phase = flexMakePhase(0, 0);
	sDesc.SkinningFalloff = 2.0f;
	sDesc.SkinningMaxDistance = 100.0f;

	sDesc.Phase = flexMakePhase(0, 0);

	int group = 1;
	for (size_t i = 0; i < 10; i++)
	{
		sDesc.Phase = flexMakePhase(group, 0);
		group++;
		FlexSoftbody* pSoftbody = new FlexSoftbody(L"./Resources/GradWork/Meshes/patrick.flux", &sDesc, m_pFlexSystem);;
		AddChild(pSoftbody);
		pSoftbody->SetTexture(L"./Resources/GradWork/Textures/patrick.jpg");
		pSoftbody->SetPosition(Vector3(randF(-1.0f, 1.0f), 15.0f + 5.0f * i, randF(-1.0f, 1.0f)));
	}


	m_pFlexSystem->InitializeSolver();
	m_pFlexSystem->UploadFlexData();
	m_pFlexSystem->UpdateSolver();
	m_pFlexMousePicker = new FlexMousePicker(m_pGameContext, m_pFlexSystem);
	m_pFlexDebugRenderer = new FlexDebugRenderer(m_pFlexSystem);
	AddChild(m_pFlexDebugRenderer);
	m_pGameContext->Scene->Input->CursorVisible(false);
}

void CollisionDemoScene::Update()
{
	if (m_FlexUpdate)
		m_pFlexSystem->UpdateSolver(1.0f / 60.0f);

	//Fetch data
	m_pFlexSystem->FetchData();
	m_pFlexMousePicker->Update();

	m_pSpinnerObject->GetTransform()->Rotate(GameTimer::DeltaTime() * m_RotateSpeed, 0, 0, Space::WORLD);
}

void CollisionDemoScene::LateUpdate()
{
	//Update data
	m_pFlexSystem->UpdateData();
}

void CollisionDemoScene::Render()
{
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Exit"))
			PostQuitMessage(0);
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();

	ImGui::Begin("Nvidia FleX");

	ImGui::Separator();
	ImGui::SliderInt("Substeps", &m_pFlexSystem->Substeps, 0, 10);
	ImGui::SliderInt("Iterations", &m_pFlexSystem->Params.mNumIterations, 0, 10);
	ImGui::Separator();
	ImGui::SliderFloat("Gravity X", &m_pFlexSystem->Params.mGravity[0], -10, 10);
	ImGui::SliderFloat("Gravity Y", &m_pFlexSystem->Params.mGravity[1], -10, 10);
	ImGui::SliderFloat("Gravity Z", &m_pFlexSystem->Params.mGravity[2], -10, 10);
	ImGui::Separator();
	ImGui::SliderFloat("Radius", &m_pFlexSystem->Params.mRadius, 0, 4);
	ImGui::SliderFloat("Soid Radius", &m_pFlexSystem->Params.mSolidRestDistance, 0, 4);
	ImGui::SliderFloat("Fluid Radius", &m_pFlexSystem->Params.mFluidRestDistance, 0, 4);
	ImGui::Separator();
	ImGui::SliderFloat("Dynamic Friction", &m_pFlexSystem->Params.mDynamicFriction, 0, 1);
	ImGui::SliderFloat("Static Friction", &m_pFlexSystem->Params.mStaticFriction, 0, 1);
	ImGui::SliderFloat("Particle Friction", &m_pFlexSystem->Params.mParticleFriction, 0, 1);
	ImGui::SliderFloat("Restitution", &m_pFlexSystem->Params.mRestitution, 0, 1);
	ImGui::SliderFloat("Damping", &m_pFlexSystem->Params.mDamping, 0, 1);
	ImGui::SliderFloat("Plastic Creep", &m_pFlexSystem->Params.mPlasticCreep, 0, 1);

	ImGui::Separator();
	ImGui::Text("Particle Count: %i", m_pFlexSystem->Positions.size());
	ImGui::Text("Rigid Count: %i", m_pFlexSystem->RigidTranslations.size());
	ImGui::Text("Collision Meshes: %i", m_pFlexSystem->ShapeGeometry.size());

	ImGui::PlotLines("Frame times", GameTimer::GetFrameTimes().data(),
		GameTimer::GetFrameTimes().size(), 1, nullptr, 0.0f, 0.05f, ImVec2(0, 80));

	ImGui::Checkbox("Simulate", &m_FlexUpdate);
	if (ImGui::Button("Restart scene"))
		GameManager::GetInstance()->ReloadScene(new CollisionDemoScene());
	if (ImGui::Button("Toggle debugging"))
		m_pFlexDebugRenderer->ToggleDebugging();

	ImGui::SliderFloat("Spinner rotate speed", &m_RotateSpeed, -100.0f, 100.0f);

	ImGui::End();
}
