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

SoftBodyScene::SoftBodyScene()
{}

SoftBodyScene::~SoftBodyScene()
{
	SafeDelete(m_pFlexMousePicker);

	flexDestroySolver(m_pFlexSolver);
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
	pGroundPlane->GetTransform()->SetPosition(0.0f, -5.0f, 0.0f);
	pGroundPlane->GetTransform()->SetScale(200.0f, 200.0f, 200.0f);
	AddChild(pGroundPlane);

	//Add a skybox
	Skybox* pSky = new Skybox();
	AddChild(pSky);

	//Flex Initialization
	flexInit(FLEX_VERSION, &FlexHelper::FlexMessageCallback);
	m_pFlexSolver = flexCreateSolver(FLEX_MAX_PARTICLES, 0);

	//Set default flex params
	FlexHelper::SetDefaultParams(m_FlexData.Params);
	FlexHelper::CreateGroundPlane(m_FlexData.Params, Vector3(0, 1, 0), -5.0f);

	m_FlexData.Params.mRadius = 0.05f;
	m_FlexData.Params.mDynamicFriction = 0.35f;
	m_FlexData.Params.mParticleFriction = 0.25f;
	m_FlexData.Params.mFluidRestDistance = m_FlexData.Params.mRadius*0.75f;
	m_FlexData.Params.mFluid = true;
	m_FlexData.Params.mNumIterations = 4;

	//Create the softbody
	SoftbodyDesc sDesc;
	sDesc.Radius = m_FlexData.Params.mRadius;
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

	/*RigidbodyDesc rDesc;
	rDesc.Radius = m_FlexData.Params.mRadius;
	rDesc.Expand = -rDesc.Radius;
	rDesc.Phase = flexMakePhase(0, 0);*/

	FlexSoftbody* pSoftbody = new FlexSoftbody(L"./Resources/Meshes/patrick.flux", &sDesc, &m_FlexData);
	AddChild(pSoftbody);
	pSoftbody->SetTexture(ResourceManager::Load<Texture>(L"./Resources/Textures/patrick.tga"));

	/*sDesc.Phase = flexMakePhase(1, 0);
	pSoftbody = new FlexSoftbody(L"./Resources/Meshes/patrick.flux", &sDesc, &m_FlexData);
	AddChild(pSoftbody);
	pSoftbody->SetTexture(ResourceManager::Load<Texture>(L"./Resources/Textures/patrick.tga"));*/

	SetFlexData();

	//Create the visual debugger
	m_pFlexDebugRenderer = new FlexDebugRenderer(m_pFlexSolver, &m_FlexData, FLEX_MAX_PARTICLES);
	AddChild(m_pFlexDebugRenderer);

	m_pGameContext->Scene->Input->AddInputAction(InputAction(FLEX_SIMULATE, Pressed, 'F'));

	m_pGameContext->Scene->Input->AddInputAction(InputAction(RESTART, Pressed, 'R'));

	m_pGameContext->Scene->Input->AddInputAction(InputAction(FLEX_TELEPORT, Pressed, 'T'));

	m_pGameContext->Scene->Input->AddInputAction(InputAction(FLEX_DEBUG, Pressed, 'O'));

	flexUpdateSolver(m_pFlexSolver, 1.0f / 60.0f, FLEX_SUBSTEPS, nullptr);
	flexGetRigidTransforms(m_pFlexSolver, (float*)m_FlexData.RigidRotations.data(), (float*)m_FlexData.RigidTranslations.data(), eFlexMemoryHost);

	m_pFlexMousePicker = new FlexMousePicker(m_pGameContext, &m_FlexData);
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
		flexUpdateSolver(m_pFlexSolver, 1.0f / 60.0f, FLEX_SUBSTEPS, nullptr);
		flexGetRigidTransforms(m_pFlexSolver, (float*)m_FlexData.RigidRotations.data(), (float*)m_FlexData.RigidTranslations.data(), eFlexMemoryHost);
	}
	if(m_pGameContext->Scene->Input->IsActionTriggered(FLEX_DEBUG))
	{
		m_pFlexDebugRenderer->ToggleDebugging();
	}
	//Fetch data
	flexGetParticles(m_pFlexSolver, (float*)m_FlexData.Positions.data(), m_FlexData.Positions.size(), m_MemoryType);
	flexGetVelocities(m_pFlexSolver, (float*)m_FlexData.Velocities.data(), m_FlexData.Velocities.size(), m_MemoryType);

	m_pFlexMousePicker->Update();
}

void SoftBodyScene::LateUpdate()
{
	//Update data
	flexSetParticles(m_pFlexSolver, (float*)m_FlexData.Positions.data(), m_FlexData.Positions.size(), m_MemoryType);
	flexSetVelocities(m_pFlexSolver, (float*)m_FlexData.Velocities.data(), m_FlexData.Velocities.size(), m_MemoryType);
	flexSetParams(m_pFlexSolver, &m_FlexData.Params);
}

void SoftBodyScene::Render()
{
}

void SoftBodyScene::SetFlexData()
{
	//Adjust the flex params
	m_FlexData.Params.mRadius *= 1.5f;
	FlexHelper::AdjustParams(m_FlexData.Params);
	flexSetParams(m_pFlexSolver, &m_FlexData.Params);

	const int numParticles = m_FlexData.Positions.size();

	flexSetParticles(m_pFlexSolver, (float*)m_FlexData.Positions.data(), numParticles, m_MemoryType);
	flexSetVelocities(m_pFlexSolver, (float*)m_FlexData.Velocities.data(), numParticles, m_MemoryType);

	vector<int> activeIndices(numParticles);
	for (int i = 0; i < numParticles; i++)
		activeIndices[i] = i;
	flexSetActive(m_pFlexSolver, activeIndices.data(), numParticles, m_MemoryType);

	//Rigids
	if (m_FlexData.RigidOffsets.size() > 1)
	{
		const int numRigids = m_FlexData.RigidOffsets.size() - 1;
		vector<Vector3> rigidLocalPositions(m_FlexData.RigidOffsets.back());

		// calculate local rest space positions
		FlexHelper::CalculateRigidOffsets(m_FlexData, numRigids, &rigidLocalPositions[0]);

		m_FlexData.RigidRotations.resize(m_FlexData.RigidOffsets.size() - 1, Vector4());
		m_FlexData.RigidTranslations.resize(m_FlexData.RigidOffsets.size() - 1, Vector3());

		flexSetRigids(
			m_pFlexSolver,
			&m_FlexData.RigidOffsets[0],
			&m_FlexData.RigidIndices[0],
			(float*)rigidLocalPositions.data(),
			nullptr,
			m_FlexData.RigidCoefficients.data(),
			(float*)&m_FlexData.RigidRotations[0],
			(float*)&m_FlexData.RigidTranslations[0],
			numRigids,
			m_MemoryType
		);
	}

	//Springs
	if (m_FlexData.SpringIndices.size() > 0)
		flexSetSprings(m_pFlexSolver, m_FlexData.SpringIndices.data(), m_FlexData.SpringLengths.data(), m_FlexData.SpringStiffness.data(), m_FlexData.SpringLengths.size(), m_MemoryType);

	flexSetPhases(m_pFlexSolver, m_FlexData.Phases.data(), numParticles, m_MemoryType);

	m_FlexData.RestPositions.insert(m_FlexData.RestPositions.begin(), m_FlexData.Positions.begin(), m_FlexData.Positions.end());
	flexSetRestParticles(m_pFlexSolver, (float*)m_FlexData.RestPositions.data(), numParticles, m_MemoryType);
}