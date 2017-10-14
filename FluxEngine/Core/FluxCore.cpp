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
#include "Context.h"
#include "Rendering/MeshFilter.h"
#include "Rendering/Camera/FreeCamera.h"
#include "Rendering/Camera/Camera.h"
#include "Rendering/Core/RasterizerState.h"
#include "Rendering/Core/BlendState.h"
#include "Rendering/Core/DepthStencilState.h"
#include "Config.h"
#include "Rendering/ParticleSystem/ParticleSystem.h"

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

	ResourceManager::Release();
	Console::Release();
	Config::Flush();
}

int FluxCore::Run(HINSTANCE hInstance)
{
	Console::Initialize();

	if (!FileSystem::Mount("./Resources.pak", "Resources", ArchiveType::Pak))
	{
		FLUX_LOG(WARNING, "Failed to mount './Resources.paK'");
	}
	if (!FileSystem::Mount("./Resources", "Resources", ArchiveType::Physical))
	{
		FLUX_LOG(WARNING, "Failed to mount './Resources.paK'");
	}

	Config::Initialize();

	m_pGraphics = make_unique<Graphics>(hInstance);
	if (!m_pGraphics->SetMode(
		Config::GetInt("Width", "Window", 1240),
		Config::GetInt("Height", "Window", 720),
		(WindowType)Config::GetInt("WindowMode", "Window", 0),
		Config::GetInt("Resizable", "Window", 1),
		Config::GetInt("VSync", "Window", 1),
		Config::GetInt("MSAA", "Window", 8),
		Config::GetInt("RefreshRate", "Window", 60)))
	{
		FLUX_LOG(ERROR, "[FluxCore::Run] > Failed to initialize graphics");
	}

	m_pGraphics->SetWindowTitle("Hello World");
	ResourceManager::Initialize(m_pGraphics.get());
	m_pInput = make_unique<InputEngine>(m_pGraphics.get());
	m_pImmediateUI = make_unique<ImmediateUI>(m_pGraphics.get(), m_pInput.get());

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

	XMFLOAT4X4 viewProj = m_pCamera->GetCamera()->GetViewProjection();
	XMMATRIX world = XMMatrixRotationY(GameTimer::GameTime()) * XMMatrixTranslation(0, 0, 5);
	XMMATRIX vp = XMLoadFloat4x4(&viewProj);
	XMMATRIX wvp = world * vp;

	m_pVertexShader->SetParameter("cWorld", &world);
	m_pVertexShader->SetParameter("cWorldViewProj", &wvp);
	m_pPixelShader->SetParameter("cColor", &m_Color);
	m_pPixelShader->SetParameter("cLightDirection", &m_LightDirection);

	Texture* pTexture = ResourceManager::Load<Texture>("Resources/Textures/spot.png");
	m_pGraphics->SetTexture(0, pTexture);

	m_pGraphics->SetShaders(m_pVertexShader, m_pPixelShader);
	m_pGraphics->SetIndexBuffer(m_pIndexBuffer);
	m_pGraphics->SetVertexBuffer(m_pVertexBuffer);
	m_pGraphics->SetInputLayout(m_pInputLayout);
	m_pGraphics->SetScissorRect(false);
	m_pGraphics->GetRasterizerState()->SetCullMode(CullMode::NONE);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::REPLACE, true);
	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::LESS);

	m_pCamera->GetCamera()->SetViewport(0, 0, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight());

	m_pGraphics->PrepareDraw();
	m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, m_IndexCount, 0, 0);

	m_pImmediateUI->NewFrame();
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("MS: %f", GameTimer::DeltaTime());
	ImGui::Text("FPS: %f", 1.0f / GameTimer::DeltaTime());
	ImGui::End();

	ImGui::ColorPicker4("Color Picker", &m_Color.x);
	ImGui::SliderFloat3("Light Direction", &m_LightDirection.x, -1, 1);

	m_pImmediateUI->Render();

	m_pGraphics->EndFrame();
}

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
};

void FluxCore::InitGame()
{

	m_pCamera = make_unique<FreeCamera>(m_pInput.get(), m_pGraphics.get());
	m_pCamera->BaseInitialize(nullptr);

	m_pShader =  new Shader(m_pGraphics.get());
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

	m_pIndexBuffer = new IndexBuffer(m_pGraphics.get());
	m_pIndexBuffer->Create((int)indexes.size());
	m_pIndexBuffer->SetData(indexes.data());

	vector<VertexElement> elements;
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::POSITION });
	elements.push_back({ VertexElementType::VECTOR2, VertexElementSemantic::TEXCOORD });
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::NORMAL });

	m_pVertexBuffer = new VertexBuffer(m_pGraphics.get());
	m_pVertexBuffer->Create((int)vertices.size(), elements);
	m_pVertexBuffer->SetData(vertices.data());

	m_pInputLayout = new InputLayout(m_pGraphics.get());
	m_pInputLayout->Create({ m_pVertexBuffer }, m_pVertexShader);

	
	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, 1, 1));
}