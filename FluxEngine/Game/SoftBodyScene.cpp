#include "stdafx.h"
#include "SoftBodyScene.h"
#include "../Scenegraph/GameObject.h"
#include "../Components/MeshComponent.h"
#include "../Prefabs/Skybox.h"

#include "../Materials/Forward/DefaultMaterial.h"
#include "../Components/TransformComponent.h"
#include <ctime>
#include "../Managers/GameManager.h"
#include "../Physics/Flex/FlexSoftbody.h"
#include "../Managers/SoundManager.h"
#include "../Physics/Flex/FlexMousePicker.h"
#include "../Physics/Flex/FlexDebugRenderer.h"
#include "../Materials/Deferred/BasicMaterial_Deferred.h"
#include "../Components/AudioSource.h"
#include "../Graphics/Texture.h"
#include "../Graphics/RenderTarget.h"
#include "../FlexSystem.h"

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
	m_pGroundMaterial->SetColor(Vector4(0.5f, 0.8f, 1.0f, 1.0f));
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

	m_pFlexSystem->CreateTriangleMesh(ResourceManager::Load<MeshFilter>(L"./Resources/Meshes/truck.flux"));
	GameObject* pPatrick = new GameObject();
	MeshComponent* pPatrickMesh = new MeshComponent(L"./Resources/Meshes/truck.flux");
	pPatrickMesh->SetMaterial(m_pGroundMaterial.get());
	pPatrick->AddComponent(pPatrickMesh);
	AddChild(pPatrick);

	//Set default flex params
	m_pFlexSystem->SetDefaultParams();
	m_pFlexSystem->CreateGroundPlane(Vector3(0, 1, 0), 0.0f);

	m_pFlexSystem->Params.mRadius = 0.05f;
	m_pFlexSystem->Params.mDynamicFriction = 0.35f;
	m_pFlexSystem->Params.mNumIterations = 4;

	//Create the softbody
	SoftbodyDesc sDesc;
	sDesc.Radius = m_pFlexSystem->Params.mRadius;
	sDesc.VolumeSampling = 4.0f;
	sDesc.SurfaceSampling = 1.0f;
	sDesc.ClusterSpacing = 2.75f;
	sDesc.ClusterRadius = 3.0f;
	sDesc.ClusterStiffness = 0.15f;
	sDesc.LinkRadius = 0.0f;
	sDesc.LinkStiffness = 1.0f;
	sDesc.Phase = flexMakePhase(0, 0);
	sDesc.SkinningFalloff = 2.0f;
	sDesc.SkinningMaxDistance = 100.0f;

	for (size_t i = 0; i < 1; i++)
	{
		sDesc.Phase = flexMakePhase(i, 0);
		FlexSoftbody* pSoftbody = new FlexSoftbody(L"./Resources/Meshes/patrick.flux", &sDesc, m_pFlexSystem);
		AddChild(pSoftbody);
		pSoftbody->SetTexture(L"./Resources/Textures/patrick.tga");
		pSoftbody->SetPosition(Vector3(0.0f, 6.0f+4*i, 0.0f));
	}

	m_pGameContext->Scene->Input->AddInputAction(InputAction(FLEX_SIMULATE, Pressed, 'F'));
	m_pGameContext->Scene->Input->AddInputAction(InputAction(RESTART, Pressed, 'R'));
	m_pGameContext->Scene->Input->AddInputAction(InputAction(FLEX_TELEPORT, Pressed, 'T'));
	m_pGameContext->Scene->Input->AddInputAction(InputAction(FLEX_DEBUG, Pressed, 'O'));

	m_pFlexSystem->UploadFlexData();
	m_pFlexSystem->Update();
	m_pFlexMousePicker = new FlexMousePicker(m_pGameContext, m_pFlexSystem);
	m_pFlexDebugRenderer = new FlexDebugRenderer(m_pFlexSystem);
	AddChild(m_pFlexDebugRenderer);
}

void SoftBodyScene::Update()
{
	if (m_pGameContext->Scene->Input->IsActionTriggered(RESTART))
		GameManager::GetInstance()->LoadScene(new SoftBodyScene());
	if (m_pGameContext->Scene->Input->IsActionTriggered(FLEX_SIMULATE))
	{
		m_FlexUpdate = !m_FlexUpdate;
	}
	if (m_FlexUpdate)
	{
		m_pFlexSystem->Update(3, 1.0f / 60.0f);
	}
	if(m_pGameContext->Scene->Input->IsActionTriggered(FLEX_DEBUG))
	{
		m_pFlexDebugRenderer->ToggleDebugging();
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
}