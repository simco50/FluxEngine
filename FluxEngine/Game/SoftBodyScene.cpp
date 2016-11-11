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
#include "../Physics/Flex/FlexSystem.h"
#include "../Physics/Flex/FlexTriangleMeshCollider.h"
#include "../Physics/Flex/FlexRigidbody.h"
#include "../UI/SpriteRenderer.h"
#include "../UI/TextRenderer.h"
#include "Objects/UI_Objects.h"
#include "../Physics/Flex/FlexHelper.h"

SoftBodyScene::SoftBodyScene()
{}

SoftBodyScene::~SoftBodyScene()
{
	SafeDelete(m_pCanvas);
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

	m_pFlexSystem->Params.mRadius = 0.05f;
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

	m_pCanvas = new UI_Canvas(m_pGameContext);
	m_pCanvas->AddElement(new UI_Image(L"./Resources/Textures/UI/Background.png", Vector2()));
	m_pCanvas->AddElement(new UI_Slider<float>(Vector2(20.0f, 50.0f), &m_pFlexSystem->Params.mGravity[0], -20, 20, L"Gravity X"));
	m_pCanvas->AddElement(new UI_Slider<float>(Vector2(20.0f, 90.0f), &m_pFlexSystem->Params.mGravity[1], -20, 20, L"Gravity Y"));
	m_pCanvas->AddElement(new UI_Slider<float>(Vector2(20.0f, 130.0f), &m_pFlexSystem->Params.mGravity[2], -20, 20, L"Gravity Z"));
	m_pCanvas->AddElement(new UI_Slider<float>(Vector2(20.0f, 170.0f), &m_pFlexSystem->Params.mPlasticCreep, 0, 1, L"Plastic Creep"));
	m_pCanvas->AddElement(new UI_Slider<float>(Vector2(20.0f, 210.0f), &m_pFlexSystem->Params.mDynamicFriction, 0.0f, 1.0f, L"Dynamic Friction"));
	m_pCanvas->AddElement(new UI_Slider<float>(Vector2(20.0f, 250.0f), &m_pFlexSystem->Params.mParticleFriction, 0.0f, 1.0f, L"Particle Friction"));
	m_pCanvas->AddElement(new UI_Slider<float>(Vector2(20.0f, 290.0f), &m_pFlexSystem->Params.mCollisionDistance, 0.01f, 4.0f, L"Collision Distance"));
	m_pCanvas->AddElement(new UI_Slider<float>(Vector2(20.0f, 330.0f), &m_pFlexSystem->Params.mRestitution, 0.0f, 1.0f, L"Restitution"));
	m_pCanvas->AddElement(new UI_Checkbox(Vector2(30.0f, 370.0f), &m_pFlexSystem->Params.mFluid, L"Fluid"));
	m_pCanvas->AddElement(new UI_Checkbox(Vector2(30.0f, 460.0f), &m_FlexUpdate, L"Simulate"));
	m_pCanvas->AddElement(new UI_Button(Vector2(20.0f, 480.0f), [this]() {m_pFlexDebugRenderer->ToggleDebugging(); }, L"Toggle debug"));
	m_pCanvas->AddElement(new UI_Button(Vector2(20.0f, 520.0f), []() {GameManager::GetInstance()->LoadScene(new SoftBodyScene()); }, L"Restart scene"));
}

void SoftBodyScene::Update()
{
	m_pCollision->GetTransform()->Rotate(100 * 0.016f, 0, 0);

	
	if (m_FlexUpdate)
	{
		m_pFlexSystem->UpdateSolver(10, 1.0f / 60);
	}
	if (m_pGameContext->Scene->Input->IsActionTriggered(FLEX_UI))
	{
		m_pCanvas->ToggleActive();
	}

	//Fetch data
	m_pFlexSystem->FetchData();
	m_pFlexMousePicker->Update();

	if (m_pCanvas->IsActive())
	{
		wstringstream stream = wstringstream();
		stream << L"Particles - " << m_pFlexSystem->Positions.size();
		TextRenderer::GetInstance()->DrawText(ResourceManager::Load<SpriteFont>(L"./Resources/Fonts/Consolas_22.fnt"), stream.str(), Vector2(20.0f, 570.0f));
		stream = wstringstream();
		stream << L"Triangle meshes - " << m_pFlexSystem->ShapeGeometry.size();
		TextRenderer::GetInstance()->DrawText(ResourceManager::Load<SpriteFont>(L"./Resources/Fonts/Consolas_22.fnt"), stream.str(), Vector2(20.0f, 590.0f));
		stream = wstringstream();
		stream << L"Rigid count - " << m_pFlexSystem->RigidTranslations.size();
		TextRenderer::GetInstance()->DrawText(ResourceManager::Load<SpriteFont>(L"./Resources/Fonts/Consolas_22.fnt"), stream.str(), Vector2(20.0f, 610.0f));
	}
}

void SoftBodyScene::LateUpdate()
{
	//Update data
	m_pFlexSystem->UpdateData();
}

void SoftBodyScene::Render()
{
	m_pCanvas->Update();
}