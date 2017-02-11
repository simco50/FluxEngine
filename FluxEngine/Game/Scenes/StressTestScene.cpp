#include "stdafx.h"
#include "StressTestScene.h"
#include "../Scenegraph/GameObject.h"
#include "../Components/MeshComponent.h"
#include "../Prefabs/Skybox.h"
#include "../Materials/Forward/DefaultMaterial.h"
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
#include "../UI/ImgUI/imgui.h"

StressTestScene::StressTestScene()
{}

StressTestScene::~StressTestScene()
{
	SafeDelete(m_pFlexMousePicker);
	SafeDelete(m_pFlexSystem);
}

void StressTestScene::Initialize()
{
	Skybox* pSkybox = new Skybox();
	AddChild(pSkybox);
	pSkybox->SetTexture(ResourceManager::Load<Texture>(L"./Resources/GradWork/Skyboxes/skybox_gray.dds"));

	m_pFlexSystem = new FlexSystem();

	m_pDefaultMaterial = make_unique<DefaultMaterial>();
	m_pDefaultMaterial->Initialize(m_pGameContext);
	m_pDefaultMaterial->SetDiffuseTexture(L"./Resources/Gradwork/Textures/stresstest_container.jpg");

	FlexTriangleMeshCollider* pTriangleMeshCollider = new FlexTriangleMeshCollider(m_pFlexSystem);
	m_pCollision = new GameObject();
	MeshComponent* pSpinnerMesh = new MeshComponent(L"./Resources/GradWork/Meshes/stresstest_container.flux");
	pSpinnerMesh->SetMaterial(m_pDefaultMaterial.get());
	m_pCollision->AddComponent(pTriangleMeshCollider);
	m_pCollision->AddComponent(pSpinnerMesh);
	AddChild(m_pCollision);

	//Set default flex params
	m_pFlexSystem->SetDefaultParams();

	m_pFlexSystem->Params.mRadius = 0.15f;
	m_pFlexSystem->Params.mDynamicFriction = 0.35f;
	m_pFlexSystem->Params.mNumIterations = 4;

	//Create the softbody
	SoftbodyDesc sDesc;
	sDesc.Radius = m_pFlexSystem->Params.mRadius;
	sDesc.VolumeSampling = 4.0f;
	sDesc.SurfaceSampling = 1.0f;
	sDesc.ClusterSpacing = 2.5f;
	sDesc.ClusterRadius = 2.7f;
	sDesc.ClusterStiffness = 0.05f;
	sDesc.LinkRadius = 0.0f;
	sDesc.LinkStiffness = 1.0f;
	sDesc.Phase = flexMakePhase(0, 0);
	sDesc.SkinningFalloff = 2.0f;
	sDesc.SkinningMaxDistance = 100.0f;

	int group = 0;
	for (size_t z = 0; z < 50; z++)
	{
		sDesc.Phase = flexMakePhase(group, 0);
		++group;
		FlexSoftbody* pSoftbody;
		int r = rand() % 4;
		switch(r)
		{
		case 0:
			pSoftbody = new FlexSoftbody(L"./Resources/GradWork/Meshes/patrick.flux", &sDesc, m_pFlexSystem);
			break;
		case 1:
			pSoftbody = new FlexSoftbody(L"./Resources/GradWork/Meshes/octopus.flux", &sDesc, m_pFlexSystem);
			break;
		case 2:
			pSoftbody = new FlexSoftbody(L"./Resources/GradWork/Meshes/armadillo.flux", &sDesc, m_pFlexSystem);
			break;
		default:
			pSoftbody = new FlexSoftbody(L"./Resources/GradWork/Meshes/teapot.flux", &sDesc, m_pFlexSystem);
			break;
		}
		AddChild(pSoftbody);
		pSoftbody->SetPosition(Vector3(0, 5.0f + z * 5.0f, 0));
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

void StressTestScene::Update()
{
	if (m_FlexUpdate)
		m_pFlexSystem->UpdateSolver(1.0f / 60.0f);

	//Fetch data
	m_pFlexSystem->FetchData();
	m_pFlexMousePicker->Update();
}

void StressTestScene::LateUpdate()
{
	//Update data
	m_pFlexSystem->UpdateData();
}

void StressTestScene::Render()
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
		
	ImGui::PlotLines("Frame times", GameTimer::GetFrameTimes().data() , 
		GameTimer::GetFrameTimes().size(), 1, nullptr, 0.0f, 0.05f, ImVec2(0, 80));

	ImGui::Checkbox("Simulate", &m_FlexUpdate);
	if (ImGui::Button("Restart scene"))
		GameManager::GetInstance()->ReloadScene(new StressTestScene());
	if (ImGui::Button("Toggle debugging"))
		m_pFlexDebugRenderer->ToggleDebugging();
	ImGui::End();
}
