#include "stdafx.h"
#include "ParticleScene.h"
#include "../Scenegraph/GameObject.h"
#include "../Components/MeshComponent.h"

#include "../Materials/Forward/DefaultMaterial.h"
#include "../Components/TransformComponent.h"
#include <ctime>
#include "../Game/GameManager.h"
#include "../Managers/SoundManager.h"
#include "../Physics/Flex/FlexMousePicker.h"
#include "../Physics/Flex/FlexDebugRenderer.h"
#include "../Materials/Deferred/BasicMaterial_Deferred.h"
#include "../Physics/Flex/FlexSystem.h"
#include "../UI/ImgUI/imgui.h"
#include "../../Helpers/MathHelp.h"

class FlexTriangleMeshCollider;

ParticleScene::ParticleScene()
{}

ParticleScene::~ParticleScene()
{
	SafeDelete(m_pFlexSystem);
}

void ParticleScene::Initialize()
{
	//Create the ground plane
	GameObject* pGroundPlane = new GameObject();
	MeshComponent* pMeshComponent = new MeshComponent(L"./Resources/Meshes/unit_plane.flux");

	m_pGroundMaterial = make_unique<DefaultMaterial>();
	m_pGroundMaterial->Initialize(m_pGameContext);
	m_pGroundMaterial->SetDiffuseTexture(L"./Resources/GradWork/Textures/gradientground.png");
	pMeshComponent->SetMaterial(m_pGroundMaterial.get());
	pGroundPlane->AddComponent(pMeshComponent);
	pGroundPlane->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	pGroundPlane->GetTransform()->SetScale(30.0f, 30.0f, 30.0f);
	AddChild(pGroundPlane);

	m_pFlexSystem = new FlexSystem();
	m_pFlexSystem->SetDefaultParams();
	m_pFlexSystem->CreateGroundPlane();

	vector<Vector4> particles;
	vector<Vector3> velocities;
	vector<int> indices;
	vector<int> phases;

	float radius = 0.5f;
	m_pFlexSystem->Params.mRadius = radius;
	m_pFlexSystem->Params.mDynamicFriction = 0.1f;
	m_pFlexSystem->Params.mNumIterations = 4;
	m_pFlexSystem->Params.mRestitution = 0.0f;

	int i = 0;
	for (int x = 0; x < 25; x++)
	{
		for (int y = 0; y < 25; y++)
		{
			for (int z = 0; z < 25; z++)
			{
				m_pFlexSystem->Positions.push_back(Vector4((float)x + randF(-0.05f, 0.05f) - 10, (float)y + 10.0f, (float)z + randF(-0.05f, 0.05f) - 10, 1.0f));
				m_pFlexSystem->Velocities.push_back(Vector3());

				m_pFlexSystem->Phases.push_back(flexMakePhase(rand() % 6, eFlexPhaseSelfCollide | eFlexPhaseFluid));
				++i;
			}
		}
	}

	m_pFlexSystem->Positions.push_back(Vector4(0, 10.0f, 0, 1.0f));
	m_pFlexSystem->Velocities.push_back(Vector3());
	m_pFlexSystem->Phases.push_back(flexMakePhase(0, 0));

	m_pFlexSystem->AdjustParams();
	m_pFlexSystem->InitializeSolver();
	m_pFlexSystem->UploadFlexData();

	m_pFlexDebugRenderer = new FlexDebugRenderer(m_pFlexSystem);
	AddChild(m_pFlexDebugRenderer);
	m_pFlexDebugRenderer->ToggleDebugging();

	m_pFlexMousePicker = new FlexMousePicker(m_pGameContext, m_pFlexSystem);
	
}

void ParticleScene::Update()
{
	if(m_FlexUpdate)
		m_pFlexSystem->UpdateSolver(1.0f / 60.0f);
	m_pFlexMousePicker->Update();
	m_pFlexSystem->FetchData();
}

void ParticleScene::LateUpdate()
{
	//Update data
	m_pFlexSystem->UpdateData();
}

void ParticleScene::Render()
{
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
		GameManager::GetInstance()->ReloadScene(new ParticleScene());
	ImGui::End();
}