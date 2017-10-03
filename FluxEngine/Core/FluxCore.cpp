#include "stdafx.h"
#include "FluxCore.h"
#include "resource.h"
#include "UI/ImmediateUI.h"
#include "Rendering/Core/Graphics.h"
#include "Rendering/Core/ShaderVariation.h"
#include "Rendering/Core/Shader.h"
#include "Rendering/Core/VertexBuffer.h"
#include "Rendering/Core/InputLayout.h"
#include "Rendering/Core/ConstantBuffer.h"
#include "Rendering/Core/IndexBuffer.h"

using namespace std;

FluxCore::FluxCore()
{
}

FluxCore::~FluxCore()
{
	SafeDelete(m_pShader);
	SafeDelete(m_pVertexBuffer);
	SafeDelete(m_pInputLayout);
	SafeDelete(m_pConstBuffer);
	SafeDelete(m_pIndexBuffer);

	SafeDelete(m_pImmediateUI);

	SafeDelete(m_pGraphics);
	ResourceManager::Release();
	Console::Release();
}

int FluxCore::Run(HINSTANCE hInstance)
{
	Console::Initialize();

	m_pGraphics = new Graphics(hInstance);
	if (!m_pGraphics->SetMode(
		/*WindowWidth*/				1240,
		/*WindowHeight*/			720,
		/*Window type*/				WindowType::WINDOWED,
		/*Resizable*/				true,
		/*Vsync*/					true,
		/*Multisample*/				8,
		/*RefreshRate denominator*/	60))
	{
		FLUX_LOG(ERROR, "[FluxCore::Run] > Failed to initialize graphics");
	}
	ResourceManager::Initialize(m_pGraphics);

	m_pImmediateUI = new ImmediateUI(m_pGraphics);

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

	float elapsed = GameTimer::GameTime();
	float deltaTime = GameTimer::DeltaTime();
	m_pConstBuffer->SetParameter(4, 4, &elapsed);
	m_pConstBuffer->SetParameter(0, 4, &deltaTime);
	m_pConstBuffer->Apply();

	m_pGraphics->Clear(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);

	Texture* pTex = ResourceManager::Load<Texture>("./Resources/Textures/Water.png");
	m_pGraphics->SetTexture(0, pTex);
	m_pGraphics->SetShaders(m_pVertexShader, m_pPixelShader);
	m_pGraphics->SetIndexBuffer(m_pIndexBuffer);
	m_pGraphics->SetVertexBuffer(m_pVertexBuffer);
	m_pGraphics->SetInputLayout(m_pInputLayout);
	m_pGraphics->SetScissorRect(false);

	m_pGraphics->PrepareDraw();
	m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, 6, 0, 0);
	m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, 6, 0, 0);

	m_pGraphics->BeginFrame();
	m_pImmediateUI->NewFrame();

	ImGui::Text("MS: %f", GameTimer::DeltaTime());
	ImGui::Text("FPS: %f", 1.0f / GameTimer::DeltaTime());

	m_pImmediateUI->Render();

	m_pGraphics->EndFrame();
}

struct V
{
	XMFLOAT3 p;
	XMFLOAT2 c;
};

void FluxCore::InitGame()
{
	m_pGraphics->SetWindowTitle("Hello World");

	m_pShader =  new Shader(m_pGraphics);
	if (m_pShader->Load("./Resources/test.hlsl"))
	m_pVertexShader = m_pShader->GetVariation(ShaderType::VertexShader);
	m_pPixelShader = m_pShader->GetVariation(ShaderType::PixelShader);

	m_pVertexBuffer = new VertexBuffer(m_pGraphics);
	
	vector<VertexElement> elements;
	elements.push_back({ VertexElementType::VECTOR3, VertexElementSemantic::POSITION });
	elements.push_back({ VertexElementType::VECTOR2, VertexElementSemantic::TEXCOORD});

	vector<V> vertices;
	vertices.push_back({ { -0.5, -0.5, 0 },{ 0,1 } });
	vertices.push_back({ { -0.5, 0.5, 0 },{ 0,0} });
	vertices.push_back({ { 0.5, -0.5, 0 },{ 1,1 } });
	vertices.push_back({ { 0.5, 0.5, 0 },{ 1,0 } });

	m_pVertexBuffer->Create((int)vertices.size(), elements);
	m_pVertexBuffer->SetData(vertices.data());

	m_pInputLayout = new InputLayout(m_pGraphics);
	m_pInputLayout->Create({ m_pVertexBuffer }, m_pVertexShader);

	m_pConstBuffer = new ConstantBuffer(m_pGraphics);
	m_pConstBuffer->SetSize(16);
	ID3D11Buffer* pBuffer = (ID3D11Buffer*)m_pConstBuffer->GetBuffer();
	m_pGraphics->GetDeviceContext()->VSSetConstantBuffers(0, 1, &pBuffer);

	m_pIndexBuffer = new IndexBuffer(m_pGraphics);
	m_pIndexBuffer->Create(6);
	vector<unsigned int> indices{ 0,1,2,1, 3, 2 };
	m_pIndexBuffer->SetData(indices.data());

	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, 1, 1));
}