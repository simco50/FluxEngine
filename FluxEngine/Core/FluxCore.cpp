#include "stdafx.h"
#include "FluxCore.h"
#include "UI/ImmediateUI.h"
#include "Rendering/Core/Graphics.h"
#include "Rendering/Core/ShaderVariation.h"
#include "Rendering/Core/Shader.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Rendering/Core/ConstantBuffer.h"
#include "Rendering/Core/IndexBuffer.h"
#include "Rendering/MeshFilter.h"
#include "Rendering/Camera/FreeCamera.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Core/RasterizerState.h"
#include "Rendering/Core/BlendState.h"
#include "Rendering/Core/DepthStencilState.h"
#include "Config.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"
#include "Rendering/Core/RenderTarget.h"
#include "Rendering/Core/Texture.h"

using namespace std;

FluxCore::FluxCore()
{
}

FluxCore::~FluxCore()
{
	SafeDelete(m_pShader);
	SafeDelete(m_pInstanceBuffer);
	SafeDelete(m_pGraphics);

	ResourceManager::Release();
	Console::Release();
	Config::Flush();
}

int FluxCore::Run(HINSTANCE hInstance)
{
	Console::Initialize();

#ifdef NDEBUG
	if (!FileSystem::Mount("./Resources.pak", "Resources", ArchiveType::Pak))
	{
		FLUX_LOG(WARNING, "Failed to mount './Resources.paK'");
	}
#else
	FLUX_LOG(WARNING, "[DEBUG] Skipped mounting './Resources.paK'");
#endif
	if (!FileSystem::Mount("./Resources", "Resources", ArchiveType::Physical))
	{
		FLUX_LOG(WARNING, "Failed to mount './Resources'");
	}

	Config::Initialize();

	m_pGraphics = new Graphics(hInstance);
	if (!m_pGraphics->SetMode(
		Config::GetInt("Width", "Window", 1240),
		Config::GetInt("Height", "Window", 720),
		(WindowType)Config::GetInt("WindowMode", "Window", 0),
		Config::GetBool("Resizable", "Window", 1),
		Config::GetBool("VSync", "Window", 1),
		Config::GetInt("MSAA", "Window", 8),
		Config::GetInt("RefreshRate", "Window", 60)))
	{
		FLUX_LOG(ERROR, "[FluxCore::Run] > Failed to initialize graphics");
	}

	m_pGraphics->SetWindowTitle(Config::GetString("Name", "Game", "FluxEngine"));
	ResourceManager::Initialize(m_pGraphics);
	m_pInput = make_unique<InputEngine>(m_pGraphics);
	m_pImmediateUI = make_unique<ImmediateUI>(m_pGraphics, m_pInput.get());

	GameTimer::Reset();

	InitGame();

	//Game loop
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if(!GameTimer::IsPaused())
				GameLoop();
			else
				Sleep(100);
		}
	}
	return (int)msg.wParam;
}

void FluxCore::GameLoop()
{
	GameTimer::Tick();
	m_pInput->Update();
	m_pCamera->BaseUpdate();

	m_pGraphics->BeginFrame();
	m_pGraphics->Clear(ClearFlags::All, XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), 1.0f, 1);

	UpdatePerFrameParameters();
	UpdatePerViewParameters();
	UpdatePerObjectParameters();

	Texture* pTexture = ResourceManager::Load<Texture>("Resources/Textures/spot.png");
	m_pGraphics->SetTexture(0, pTexture);

	m_pGraphics->SetShader(ShaderType::VertexShader, m_pVertexShader);
	m_pGraphics->SetShader(ShaderType::PixelShader, m_pPixelShader);
	m_pGraphics->SetIndexBuffer(m_pMeshFilter->GetIndexBuffer());
	m_pGraphics->SetVertexBuffers({ m_pMeshFilter->GetVertexBuffer(0), m_pInstanceBuffer });
	m_pGraphics->SetScissorRect(false);
	m_pGraphics->GetRasterizerState()->SetCullMode(CullMode::BACK);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::REPLACE, false);
	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::LESS);

	m_pCamera->GetCamera()->SetViewport(0, 0, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight());

	m_pGraphics->DrawIndexedInstanced(PrimitiveType::TRIANGLELIST, m_pMeshFilter->GetIndexCount(), 0, m_pInstanceBuffer->GetVertexCount());

	RenderUI();

	m_pGraphics->EndFrame();
}

void FluxCore::UpdatePerFrameParameters()
{
	float dt = GameTimer::DeltaTime();
	float gt = GameTimer::GameTime();
	m_pVertexShader->SetParameter("cDeltaTimeVS", &dt);
	m_pVertexShader->SetParameter("cElapsedTimeVS", &gt);

	m_pPixelShader->SetParameter("cDeltaTimePS", &gt);
	m_pPixelShader->SetParameter("cLightDirectionPS", &m_LightDirection);
}

void FluxCore::UpdatePerObjectParameters()
{
	XMFLOAT4X4 viewProj = m_pCamera->GetCamera()->GetViewProjection();
	XMMATRIX vp = XMLoadFloat4x4(&viewProj);
	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX wvp = world * vp;
	m_pVertexShader->SetParameter("cWorldVS", &world);
	m_pVertexShader->SetParameter("cWorldViewProjVS", &wvp);

	m_pPixelShader->SetParameter("cColorPS", &m_Color);
	m_pPixelShader->SetParameter("cWorldPS", &world);
}

void FluxCore::UpdatePerViewParameters()
{
	XMFLOAT4X4 viewProj = m_pCamera->GetCamera()->GetViewProjection();
	XMMATRIX vp = XMLoadFloat4x4(&viewProj);
	m_pVertexShader->SetParameter("cViewProjVS", &vp);
	XMFLOAT4X4 view = m_pCamera->GetCamera()->GetView();
	m_pVertexShader->SetParameter("cViewVS", &view);
	XMFLOAT4X4 viewInv = m_pCamera->GetCamera()->GetViewInverse();
	m_pVertexShader->SetParameter("cViewInverseVS", &viewInv);
	float nearClip = m_pCamera->GetCamera()->GetNearPlane();
	m_pVertexShader->SetParameter("cNearClipVS", &nearClip);
	float farClip = m_pCamera->GetCamera()->GetFarPlane();
	m_pVertexShader->SetParameter("cFarClipVS", &farClip);

	m_pPixelShader->SetParameter("cViewProjPS", &vp);
	m_pPixelShader->SetParameter("cViewPS", &view);
	m_pPixelShader->SetParameter("cViewInversePS", &viewInv);
}

void FluxCore::RenderUI()
{
	m_pImmediateUI->NewFrame();

	unsigned int batchCount, primitiveCount;
	m_pGraphics->GetDebugInfo(batchCount, primitiveCount);

	stringstream timeStr;
	timeStr << setw(2) << setfill('0') << (int)GameTimer::GameTime() / 60 << ":" << setw(2) << (int)GameTimer::GameTime() % 60;
	string time = timeStr.str();
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Game Time : %s", time.c_str());
	ImGui::Text("MS: %f", GameTimer::DeltaTime());
	ImGui::Text("FPS: %f", 1.0f / GameTimer::DeltaTime());
	ImGui::Text("Primitives: %i", primitiveCount);
	ImGui::Text("Batches: %i", batchCount);
	ImGui::End();

	ImGui::ColorPicker4("Color Picker", &m_Color.x);
	ImGui::SliderFloat3("Light Direction", &m_LightDirection.x, -1, 1);

	m_pImmediateUI->Render();
}

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
};

void FluxCore::InitGame()
{
	m_pCamera = make_unique<FreeCamera>(m_pInput.get(), m_pGraphics);
	m_pCamera->BaseInitialize(nullptr);

	m_pShader =  new Shader(m_pGraphics);
	if (m_pShader->Load("Resources/Shaders/Diffuse.hlsl"))
	{
		m_pVertexShader = m_pShader->GetVariation(ShaderType::VertexShader);
		m_pPixelShader = m_pShader->GetVariation(ShaderType::PixelShader, "TEST");
	}

	//MeshFilter
	vector<VertexElement> elements;
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::POSITION });
	elements.push_back({ VertexElementType::VECTOR2, VertexElementSemantic::TEXCOORD });
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::NORMAL });
	m_pMeshFilter = ResourceManager::Load<MeshFilter>("Resources/Meshes/cube.flux");
	m_pMeshFilter->CreateBuffers(m_pGraphics, elements);

	//InstanceBuffer
	float spacing = 1.5f;
	int count = 40;
	vector<XMFLOAT3> instancePos;
	for (int x = 0; x < count; ++x)
	{
		for (int y = 0; y < count; ++y)
		{
			for (int z = 0; z < count; ++z)
			{
				instancePos.push_back(XMFLOAT3((-(float)count / 2.0f + x) * spacing, (-(float)count / 2.0f + y) * spacing, (2 + z) * spacing));
			}
		}
	}
	elements.clear();
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::TEXCOORD, 1, true });

	m_pInstanceBuffer = new VertexBuffer(m_pGraphics);
	m_pInstanceBuffer->Create((unsigned int)instancePos.size(), elements);
	m_pInstanceBuffer->SetData(instancePos.data());
	
	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, 1, 1), true);
}