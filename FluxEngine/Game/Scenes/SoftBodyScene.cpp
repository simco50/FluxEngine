#include "stdafx.h"
#include "SoftBodyScene.h"
#include "../Scenegraph/GameObject.h"
#include "../Components/MeshComponent.h"
#include "../Prefabs/Skybox.h"

#include "../Materials/Forward/DefaultMaterial.h"
#include "../Components/TransformComponent.h"
#include <ctime>
#include "../Game/GameManager.h"
#include "../Physics/Flex/FlexSoftbody.h"
#include "../Managers/SoundManager.h"
#include "../Physics/Flex/FlexMousePicker.h"
#include "../Physics/Flex/FlexDebugRenderer.h"
#include "../Materials/Deferred/BasicMaterial_Deferred.h"
#include "../Physics/Flex/FlexSystem.h"
#include "../Physics/Flex/FlexTriangleMeshCollider.h"
#include "../Physics/Flex/FlexRigidbody.h"
#include "../Physics/Flex/FlexHelper.h"
#include "../Graphics/MeshFilter.h"
#include "../UI/ImgUI/imgui.h"

SoftBodyScene::SoftBodyScene()
{}

SoftBodyScene::~SoftBodyScene()
{
	SafeDelete(m_pFlexMousePicker);
	SafeDelete(m_pFlexSystem);


	flexShutdown();
}

void SoftBodyScene::Initialize()
{
	//Create the ground plane
	GameObject* pGroundPlane = new GameObject();
	MeshComponent* pMeshComponent = new MeshComponent(L"./Resources/Meshes/unit_plane.flux");

	m_pGroundMaterial = make_unique<DefaultMaterial>();
	m_pGroundMaterial->Initialize(m_pGameContext);
	m_pGroundMaterial->SetColor(Vector4(0.0f, 0.8f, 0.6f, 1.0f));
	pMeshComponent->SetMaterial(m_pGroundMaterial.get());
	pGroundPlane->AddComponent(pMeshComponent);
	pGroundPlane->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	pGroundPlane->GetTransform()->SetScale(200.0f, 200.0f, 200.0f);
	AddChild(pGroundPlane);

	//Add a skybox
	Skybox* pSky = new Skybox();
	AddChild(pSky);

	flexInit(FLEX_VERSION, FlexHelper::FlexMessageCallback);
	m_pFlexSystem = new FlexSystem();

	m_pDefaultMaterial = make_unique<DefaultMaterial>();
	m_pDefaultMaterial->Initialize(m_pGameContext);
	m_pDefaultMaterial->SetColor(Vector4(0.7f, 0.7f, 0.7f, 1.0f));
	FlexTriangleMeshCollider* pTriangleMeshCollider = new FlexTriangleMeshCollider(m_pFlexSystem);
	m_pCollision = new GameObject();
	MeshComponent* pSpinnerMesh = new MeshComponent(L"./Resources/Meshes/spinner.flux");
	pSpinnerMesh->SetMaterial(m_pDefaultMaterial.get());
	m_pCollision->AddComponent(pTriangleMeshCollider);
	m_pCollision->AddComponent(pSpinnerMesh);
	AddChild(m_pCollision);
	m_pCollision->GetTransform()->SetPosition(0, 5, 0);

	GameObject* pContainer = new GameObject();
	pTriangleMeshCollider = new FlexTriangleMeshCollider(m_pFlexSystem);
	pSpinnerMesh = new MeshComponent(L"./Resources/Meshes/container.flux");
	pSpinnerMesh->SetMaterial(m_pDefaultMaterial.get());
	pContainer->AddComponent(pTriangleMeshCollider);
	pContainer->AddComponent(pSpinnerMesh);
	AddChild(pContainer);
	pContainer->GetTransform()->SetPosition(0, 5, 0);

	//Set default flex params
	m_pFlexSystem->SetDefaultParams();
	m_pFlexSystem->CreateGroundPlane(Vector3(0, 1, 0), 0.0f);

	m_pFlexSystem->Params.mRadius = 0.06f;
	m_pFlexSystem->Params.mDynamicFriction = 0.35f;
	m_pFlexSystem->Params.mNumIterations = 4;

	//Create the softbody
	SoftbodyDesc sDesc;
	sDesc.Radius = m_pFlexSystem->Params.mRadius;
	sDesc.VolumeSampling = 4.0f;
	sDesc.SurfaceSampling = 1.0f;
	sDesc.ClusterSpacing = 2.75f;
	sDesc.ClusterRadius = 2.5f;
	sDesc.ClusterStiffness = 0.05f;
	sDesc.LinkRadius = 0.0f;
	sDesc.LinkStiffness = 1.0f;
	sDesc.Phase = flexMakePhase(0, 0);
	sDesc.SkinningFalloff = 2.0f;
	sDesc.SkinningMaxDistance = 100.0f;

	/*MeshFilter* pDragon = ResourceManager::Load<MeshFilter>(L"./Resources/Meshes/dragon.flux");
	Vector3* pos = (Vector3*)pDragon->GetVertexData("POSITION").pData;
	for (size_t i = 0; i < pDragon->VertexCount(); i++)
	{
		pos[i] = pos[i] * 10;
	}

	for (size_t i = 0; i < 5; i++)
	{
		sDesc.Phase = flexMakePhase(i, 0);
		FlexSoftbody* pSoftbody = new FlexSoftbody(L"./Resources/Meshes/dragon.flux", &sDesc, m_pFlexSystem);
		AddChild(pSoftbody);
		//pSoftbody->SetTexture(L"./Resources/Textures/Patrick.tga");
		pSoftbody->SetPosition(Vector3(0.0f, 4.0f * i + 10.0f, 0.0f));
	}*/

	for (size_t i = 0; i < 5; i++)
	{
		sDesc.Phase = flexMakePhase(i, 0);
		FlexSoftbody* pSoftbody = new FlexSoftbody(L"./Resources/Meshes/Patrick.flux", &sDesc, m_pFlexSystem);
		AddChild(pSoftbody);
		pSoftbody->SetTexture(L"./Resources/Textures/Patrick.tga");
		pSoftbody->SetPosition(Vector3(0.0f, 4.0f * i + 10.0f, 0.0f));
	}
	
	m_pGameContext->Scene->Input->AddInputAction(InputAction(FLEX_UI, Pressed, 'U'));

	m_pFlexSystem->InitializeSolver();
	m_pFlexSystem->UploadFlexData();
	m_pFlexSystem->UpdateSolver();
	m_pFlexMousePicker = new FlexMousePicker(m_pGameContext, m_pFlexSystem);
	m_pFlexDebugRenderer = new FlexDebugRenderer(m_pFlexSystem);
	AddChild(m_pFlexDebugRenderer);

	m_pGameContext->Scene->Input->CursorVisible(false);

}

void SoftBodyScene::Update()
{
	m_pCollision->GetTransform()->Rotate(100 * 0.016f, 0, 0);
	
	if (m_FlexUpdate)
	{
		m_pFlexSystem->UpdateSolver(1.0f / 60);
	}

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
	//m_pCanvas->Update();

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
		
	ImGui::PlotLines("Frame times", GameTimer::GetFrameTimes().data() , GameTimer::GetFrameTimes().size(), 0.2f, nullptr, 0.0f, 0.05f, ImVec2(0, 80));

	ImGui::Checkbox("Simulate", &m_FlexUpdate);
	if (ImGui::Button("Restart scene"))
		GameManager::GetInstance()->LoadScene(new SoftBodyScene());
	if (ImGui::Button("Toggle debugging"))
		m_pFlexDebugRenderer->ToggleDebugging();
	ImGui::End();
}