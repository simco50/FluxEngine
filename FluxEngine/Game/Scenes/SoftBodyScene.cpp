#include "stdafx.h"
#include "SoftBodyScene.h"
#include "../../Scenegraph/GameObject.h"
#include "../../Components/MeshComponent.h"
#include "../../Prefabs/Skybox.h"

#include "../../Materials/Forward/DefaultMaterial.h"
#include "../../Components/TransformComponent.h"
#include "../../Game/GameManager.h"
#include "../../Physics/Flex/FlexSoftbody.h"
#include "../../Managers/SoundManager.h"
#include "../../Physics/Flex/FlexMousePicker.h"
#include "../../Physics/Flex/FlexDebugRenderer.h"
#include "../../Materials/Deferred/BasicMaterial_Deferred.h"
#include "../../Physics/Flex/FlexSystem.h"
#include "../../Physics/Flex/FlexTriangleMeshCollider.h"
#include "../../Physics/Flex/FlexRigidbody.h"
#include "../../UI/ImgUI/imgui.h"
#include "../../Components/CameraComponent.h"
#include "Graphics/Texture.h"

SoftBodyScene::SoftBodyScene()
{}

SoftBodyScene::~SoftBodyScene()
{
	SafeDelete(m_pFlexMousePicker);
	SafeDelete(m_pFlexSystem);
}

void SoftBodyScene::Initialize()
{
	//Add a skybox
	Skybox* pSky = new Skybox();
	AddChild(pSky);
	pSky->SetTexture(ResourceManager::Load<Texture>(L"./Resources/GradWork/Skyboxes/skybox_spongebob.dds"));

	//Create the ground plane
	GameObject* pGroundPlane = new GameObject();
	MeshComponent* pMeshComponent = new MeshComponent(L"./Resources/Meshes/unit_plane.flux");

	m_pGroundMaterial = make_unique<DefaultMaterial>();
	m_pGroundMaterial->SetDiffuseTexture(L"./Resources/GradWork/textures/ground_spongebob.png");
	m_pGroundMaterial->Initialize(m_pGameContext);
	pMeshComponent->SetMaterial(m_pGroundMaterial.get());
	pGroundPlane->AddComponent(pMeshComponent);
	pGroundPlane->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	pGroundPlane->GetTransform()->SetScale(30.0f, 30.0f, 30.0f);
	AddChild(pGroundPlane);

	m_pFlexSystem = new FlexSystem();
	InitializeMaterials();

	//Patrick house
	FlexTriangleMeshCollider* pTriangleMeshCollider = new FlexTriangleMeshCollider(m_pFlexSystem);
	GameObject* pCollisionObject = new GameObject();
	MeshComponent* pMesh = new MeshComponent(L"./Resources/GradWork/Meshes/patrick_house.flux");
	pMesh->SetMaterial(m_pMaterials[PATRICK_HOUSE]);
	pCollisionObject->AddComponent(pTriangleMeshCollider);
	pCollisionObject->AddComponent(pMesh);
	AddChild(pCollisionObject);
	pCollisionObject->GetTransform()->Translate(-10, 0, 0);

	//Spongebob house
	pTriangleMeshCollider = new FlexTriangleMeshCollider(m_pFlexSystem);
	pCollisionObject = new GameObject();
	pMesh = new MeshComponent(L"./Resources/GradWork/Meshes/squidward_house.flux");
	pMesh->SetMaterial(m_pMaterials[SQUIDWARD_HOUSE]);
	pCollisionObject->AddComponent(pTriangleMeshCollider);
	pCollisionObject->AddComponent(pMesh);
	AddChild(pCollisionObject);
	pCollisionObject->GetTransform()->Translate(0, 0, 0);

	//Squidward house
	pTriangleMeshCollider = new FlexTriangleMeshCollider(m_pFlexSystem);
	pCollisionObject = new GameObject();
	pMesh = new MeshComponent(L"./Resources/GradWork/Meshes/spongebob_house.flux");
	pMesh->SetMaterial(m_pMaterials[SPONGEBOB_HOUSE]);
	pCollisionObject->AddComponent(pTriangleMeshCollider);
	pCollisionObject->AddComponent(pMesh);
	AddChild(pCollisionObject);
	pCollisionObject->GetTransform()->Translate(10, 0, 0);
	//Set default flex params
	m_pFlexSystem->SetDefaultParams();
	m_pFlexSystem->CreateGroundPlane(Vector3(0, 1, 0), 0.0f);

	m_pFlexSystem->Params.mRadius = 0.07f;
	m_pFlexSystem->Params.mDynamicFriction = 0.35f;
	m_pFlexSystem->Params.mNumIterations = 4;

	//Create the softbody
	SoftbodyDesc sDesc;
	sDesc.Radius = m_pFlexSystem->Params.mRadius;
	sDesc.VolumeSampling = 4.0f;
	sDesc.SurfaceSampling = 1.0f;
	sDesc.ClusterSpacing = 3.0f;
	sDesc.ClusterRadius = 2.7f;
	sDesc.ClusterStiffness = 0.05f;
	sDesc.LinkRadius = 0.0f;
	sDesc.LinkStiffness = 1.0f;
	sDesc.Phase = flexMakePhase(0, 0);
	sDesc.SkinningFalloff = 2.0f;
	sDesc.SkinningMaxDistance = 100.0f;
	sDesc.Phase = flexMakePhase(0, 0);
	FlexSoftbody* pSoftbody = new FlexSoftbody(L"./Resources/gradwork/Meshes/patrick.flux", &sDesc, m_pFlexSystem);
	AddChild(pSoftbody);
	pSoftbody->SetTexture(L"./Resources/gradwork/textures/patrick.jpg");
	pSoftbody->SetPosition(Vector3(0, 9.0f, -5.0f));

	sDesc.ClusterRadius = 3.0f;
	sDesc.ClusterStiffness = 0.2f;
	sDesc.Phase = flexMakePhase(0, 0);
	pSoftbody = new FlexSoftbody(L"./Resources/gradwork/Meshes/patrick.flux", &sDesc, m_pFlexSystem);
	AddChild(pSoftbody);
	pSoftbody->SetTexture(L"./Resources/gradwork/textures/patrick_orange.jpg");
	pSoftbody->SetPosition(Vector3(0, 12.0f, -5.0f));

	sDesc.ClusterRadius = 3.0f;
	sDesc.ClusterStiffness = 0.8f;
	sDesc.Phase = flexMakePhase(0, 0);
	pSoftbody = new FlexSoftbody(L"./Resources/gradwork/Meshes/patrick.flux", &sDesc, m_pFlexSystem);
	AddChild(pSoftbody);
	pSoftbody->SetTexture(L"./Resources/gradwork/textures/patrick_red.jpg");
	pSoftbody->SetPosition(Vector3(0, 15.0f, -5.0f));

	m_pGameContext->Scene->Input->AddInputAction(InputAction(FLEX_UI, Pressed, 'U'));

	m_pFlexSystem->InitializeSolver();
	m_pFlexSystem->UploadFlexData();
	m_pFlexSystem->UpdateSolver();
	m_pFlexMousePicker = new FlexMousePicker(m_pGameContext, m_pFlexSystem);
	m_pFlexDebugRenderer = new FlexDebugRenderer(m_pFlexSystem);
	AddChild(m_pFlexDebugRenderer);

	m_pGameContext->Scene->Input->CursorVisible(false);
}

void SoftBodyScene::InitializeMaterials()
{
	DefaultMaterial* pMaterial = new DefaultMaterial();
	pMaterial->Initialize(m_pGameContext);
	pMaterial->SetDiffuseTexture(L"./Resources/GradWork/Textures/Patrick_House.jpg");
	m_pMaterials.push_back(pMaterial);

	pMaterial = new DefaultMaterial();
	pMaterial->Initialize(m_pGameContext);
	pMaterial->SetDiffuseTexture(L"./Resources/GradWork/Textures/Spongebob_House.jpg");
	m_pMaterials.push_back(pMaterial);

	pMaterial = new DefaultMaterial();
	pMaterial->Initialize(m_pGameContext);
	pMaterial->SetDiffuseTexture(L"./Resources/GradWork/Textures/Squidward_House.jpg");
	m_pMaterials.push_back(pMaterial);
}

void SoftBodyScene::Update()
{
	if (m_FlexUpdate)
		m_pFlexSystem->UpdateSolver(1.0f / 60);

	//Fetch data
	m_pFlexSystem->FetchData();
	m_pFlexMousePicker->Update();
}

void SoftBodyScene::LateUpdate()
{
	//Update data
	m_pFlexSystem->UpdateData();
}

void SoftBodyScene::Render()
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
	ImGui::SliderFloat("Adhesion", &m_pFlexSystem->Params.mAdhesion, 0, 2);

	ImGui::Separator();
	ImGui::Text("Particle Count: %i", m_pFlexSystem->Positions.size());
	ImGui::Text("Rigid Count: %i", m_pFlexSystem->RigidTranslations.size());
	ImGui::Text("Collision Meshes: %i", m_pFlexSystem->ShapeGeometry.size());

	ImGui::PlotLines("Frame times", GameTimer::GetFrameTimes().data() ,
		GameTimer::GetFrameTimes().size(), 1, nullptr, 0.0f, 0.05f, ImVec2(0, 80));

	ImGui::Checkbox("Simulate", &m_FlexUpdate);
	if (ImGui::Button("Restart scene"))
		GameManager::GetInstance()->ReloadScene(new SoftBodyScene());
	if (ImGui::Button("Toggle debugging"))
		m_pFlexDebugRenderer->ToggleDebugging();
	ImGui::End();
}
