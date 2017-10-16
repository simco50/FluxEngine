#include "stdafx.h"
#include "FluxCore.h"
#include "UI/ImmediateUI.h"
#include "Rendering/Core/Graphics.h"
#include "Rendering/Core/ShaderVariation.h"
#include "Rendering/Core/Shader.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Rendering/Core/InputLayout.h"
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

using namespace std;

FluxCore::FluxCore()
{
}

FluxCore::~FluxCore()
{
	SafeDelete(m_pShader);
	SafeDelete(m_pVertexBuffer);
	SafeDelete(m_pInputLayout);
	SafeDelete(m_pIndexBuffer);
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
		FLUX_LOG(WARNING, "Failed to mount './Resources.paK'");
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
	m_pGraphics->Clear(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, (XMFLOAT4)DirectX::Colors::CornflowerBlue, 1.0f, 1);

#pragma region
	float dt = GameTimer::DeltaTime();
	float gt = GameTimer::GameTime();
	m_pVertexShader->SetParameter("cDeltaTimeVS", &dt);
	m_pVertexShader->SetParameter("cElapsedTimeVS", &gt);

	m_pPixelShader->SetParameter("cDeltaTimePS", &gt);
	m_pPixelShader->SetParameter("cLightDirectionPS", &m_LightDirection);
#pragma endregion PER_FRAME

#pragma region
	XMFLOAT4X4 viewProj = m_pCamera->GetCamera()->GetViewProjection();
	XMMATRIX vp = XMLoadFloat4x4(&viewProj);
	/*m_pVertexShader->SetParameter("cViewProjVS", &vp);
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
	m_pPixelShader->SetParameter("cViewInversePS", &viewInv);*/
#pragma endregion PER_VIEW

#pragma region
	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX wvp = world * vp;
	m_pVertexShader->SetParameter("cWorldVS", &world);
	m_pVertexShader->SetParameter("cWorldViewProjVS", &wvp);

	m_pPixelShader->SetParameter("cColorPS", &m_Color);
	//m_pPixelShader->SetParameter("cWorldPS", &world);
#pragma endregion PER_OBJECT

	Texture* pTexture = ResourceManager::Load<Texture>("Resources/Textures/spot.png");
	m_pGraphics->SetTexture(0, pTexture);

	m_pGraphics->SetShaders(m_pVertexShader, m_pPixelShader);
	m_pGraphics->SetIndexBuffer(m_pIndexBuffer);
	m_pGraphics->SetVertexBuffers({ m_pVertexBuffer, m_pInstanceBuffer });
	m_pGraphics->SetInputLayout(m_pInputLayout);
	m_pGraphics->SetScissorRect(false);
	m_pGraphics->GetRasterizerState()->SetCullMode(CullMode::BACK);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::REPLACE, true);
	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::LESS);

	m_pCamera->GetCamera()->SetViewport(0, 0, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight());

	m_pGraphics->PrepareDraw();

	m_pGraphics->DrawIndexedInstanced(PrimitiveType::TRIANGLELIST, m_IndexCount, 0, 3375);

	RenderUI();

	m_pGraphics->EndFrame();
}

void FluxCore::RenderUI()
{
	m_pImmediateUI->NewFrame();

	stringstream timeStr;
	timeStr << setw(2) << setfill('0') << (int)GameTimer::GameTime() / 60 << ":" << setw(2) << (int)GameTimer::GameTime() % 60;
	string time = timeStr.str();
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Game Time : %s", time.c_str());
	ImGui::Text("MS: %f", GameTimer::DeltaTime());
	ImGui::Text("FPS: %f", 1.0f / GameTimer::DeltaTime());
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
	m_pVertexShader = m_pShader->GetVariation(ShaderType::VertexShader);
	m_pPixelShader = m_pShader->GetVariation(ShaderType::PixelShader, "TEST");

	MeshFilter* pMesh = ResourceManager::Load<MeshFilter>("Resources/Meshes/spot.flux");

	vector<Vertex> vertices;
	MeshFilter::VertexData positions = pMesh->GetVertexData("POSITION");
	MeshFilter::VertexData normals = pMesh->GetVertexData("NORMAL");
	MeshFilter::VertexData texCoord = pMesh->GetVertexData("TEXCOORD");
	for (int i = 0; i < positions.Count; ++i)
	{
		vertices.push_back({ ((XMFLOAT3*)positions.pData)[i],((XMFLOAT2*)texCoord.pData)[i],((XMFLOAT3*)normals.pData)[i] });
	}

	vector<unsigned int> indexes;
	MeshFilter::VertexData indices = pMesh->GetVertexData("INDEX");
	for (int i = 0; i < indices.Count; ++i)
	{
		indexes.push_back(((unsigned int*)indices.pData)[i]);
	}
	m_IndexCount = (int)indexes.size();

	m_pIndexBuffer = new IndexBuffer(m_pGraphics);
	m_pIndexBuffer->Create((int)indexes.size());
	m_pIndexBuffer->SetData(indexes.data());

	//Vertexbuffer
	vector<VertexElement> elements;
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::POSITION });
	elements.push_back({ VertexElementType::VECTOR2, VertexElementSemantic::TEXCOORD });
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::NORMAL });

	m_pVertexBuffer = new VertexBuffer(m_pGraphics);
	m_pVertexBuffer->Create((int)vertices.size(), elements);
	m_pVertexBuffer->SetData(vertices.data());

	elements.clear();
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::TEXCOORD, 1, true});

	m_pInstanceBuffer = new VertexBuffer(m_pGraphics);
	m_pInstanceBuffer->Create((int)vertices.size(), elements);
	m_pInstanceBuffer->SetData(vertices.data());

	vector<XMFLOAT3> instancePos;
	for (int x = 0; x < 15; ++x)
	{
		for (int y = 0; y < 15; ++y)
		{
			for (int z = 0; z < 15; ++z)
			{
				instancePos.push_back(XMFLOAT3((-7.5f + x) * 2.5f, (-7.5f + y) * 2.5f, (2 + z) * 2.5f));
			}
		}
	}
	m_pInstanceBuffer->SetData(instancePos.data());
	
	m_pInputLayout = new InputLayout(m_pGraphics);
	m_pInputLayout->Create({ m_pVertexBuffer, m_pInstanceBuffer }, m_pVertexShader);
	
	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, 1, 1), true);
}