#include "stdafx.h"
#include "ImgUIDrawer.h"
#include "Rendering\Core\Graphics.h"
#include "Rendering\Core\VertexBuffer.h"
#include "Rendering\Core\IndexBuffer.h"
#include "Rendering\Core\Shader.h"
#include "Rendering\Core\InputLayout.h"
#include "Rendering\Core\ConstantBuffer.h"

ImgUIDrawer::ImgUIDrawer(Graphics* pGraphics) :
	m_pGraphics(pGraphics)
{
}

void ImgUIDrawer::Initialize()
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.RenderDrawListsFn = nullptr;

	io.ImeWindowHandle = m_pGraphics->GetWindow();

	LoadShader();
	CreateVertexBuffer();
	CreateIndexBuffer();

	m_pIl = new InputLayout(m_pGraphics);
	m_pIl->Create({ m_pVb }, m_pVertexShader);
	CreateFontsTexture();

	m_pConstantBuffer = new ConstantBuffer(m_pGraphics);
	m_pConstantBuffer->SetSize(64);
}

void ImgUIDrawer::Shutdown()
{
	ImGui::Shutdown();
}

void ImgUIDrawer::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();

	RECT rect;
	GetClientRect(m_pGraphics->GetWindow(), &rect);
	io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	io.DeltaTime = GameTimer::DeltaTime();
	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	io.MouseDrawCursor = true;

	ImGui::NewFrame();
}

void ImgUIDrawer::Render()
{
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	if(m_pVb == nullptr || (int)m_pVb->GetCount() < draw_data->TotalVtxCount)
	{
		m_pVb->Create(m_pVb->GetCount() + 5000, m_VertexElements, true);
	}
	
	if (m_pIb == nullptr || (int)m_pIb->GetCount() < draw_data->TotalIdxCount)
	{
		CreateIndexBuffer();
		m_pIb->Create(m_pIb->GetCount() + 10000, true, true);
	}

	void* pVertexResource = m_pVb->Map(true);
	void* pIndexResource = m_pIb->Map(true);

	ImDrawVert* vtx_dst = (ImDrawVert*)pVertexResource;
	ImDrawIdx* idx_dst = (ImDrawIdx*)pIndexResource;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}

	m_pVb->Unmap();
	m_pIb->Unmap();

	m_pGraphics->SetInputLayout(m_pIl);
	m_pGraphics->SetIndexBuffer(m_pIb);
	m_pGraphics->SetVertexBuffer(m_pVb);
	m_pGraphics->SetShaders(m_pVertexShader, m_pPixelShader);

	m_pGraphics->SetDepthEnabled(true);
	m_pGraphics->SetColorWrite(ColorWrite::ALL);
	m_pGraphics->SetDepthTest(CompareMode::ALWAYS);
	m_pGraphics->SetBlendMode(BlendMode::REPLACE, false);

	m_pGraphics->SetCullMode(CullMode::NONE);

	float L = 0.0f;
	float R = ImGui::GetIO().DisplaySize.x;
	float B = ImGui::GetIO().DisplaySize.y;
	float T = 0.0f;
	float mvp[4][4] =
	{
		{ 2.0f / (R - L),		0.0f,				0.0f,       0.0f },
		{ 0.0f,					2.0f / (T - B),     0.0f,       0.0f },
		{ 0.0f,					0.0f,				0.5f,       0.0f },
		{ (R + L) / (L - R),	(T + B) / (B - T),  0.5f,       1.0f },
	};

	m_pConstantBuffer->SetParameter(0, 64, mvp);
	m_pConstantBuffer->Apply();

	ID3D11Buffer* pBuffer = (ID3D11Buffer*)m_pConstantBuffer->GetBuffer();
	m_pGraphics->GetDeviceContext()->VSSetConstantBuffers(1, 1, &pBuffer);
	m_pGraphics->PrepareDraw();

	int vertexOffset = 0;
	int indexOffset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			else
			{
				//Set the texture
				//Set the scissor rect
				m_pGraphics->Draw(PrimitiveType::TRIANGLELIST, indexOffset, pcmd->ElemCount, vertexOffset, 0);
			}
			indexOffset += pcmd->ElemCount;
		}
		vertexOffset += cmd_list->VtxBuffer.Size;
	}
}

void ImgUIDrawer::OnResize()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
}

int ImgUIDrawer::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (message)
	{
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		return 0;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return 0;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		return 0;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return 0;
	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		return 0;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		return 0;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return 0;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return 0;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return 0;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return 0;
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
	default:
		break;
	}

	return 0;
}


void ImgUIDrawer::CreateVertexBuffer()
{
	SafeDelete(m_pVb);
	m_pVb = new VertexBuffer(m_pGraphics);
	m_VertexElements.push_back(VertexElement(VertexElementType::VECTOR2, VertexElementSemantic::POSITION));
	m_VertexElements.push_back(VertexElement(VertexElementType::VECTOR2, VertexElementSemantic::TEXCOORD));
	m_VertexElements.push_back(VertexElement(VertexElementType::UBYTE4_NORM, VertexElementSemantic::COLOR));
	m_pVb->Create(1000, m_VertexElements, true);
}

void ImgUIDrawer::CreateIndexBuffer()
{
	SafeDelete(m_pIb);
	m_pIb = new IndexBuffer(m_pGraphics);
	m_pIb->Create(1000, true, true);
}

void ImgUIDrawer::CreateFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();
	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D *pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = pixels;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	HR(m_pGraphics->GetDevice()->CreateTexture2D(&desc, &subResource, &pTexture))

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(m_pGraphics->GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, m_pFontSRV.GetAddressOf()))
	pTexture->Release();

	io.Fonts->TexID = (void*)m_pFontSRV.Get();
}

void ImgUIDrawer::LoadShader()
{
	m_pShader = new Shader(m_pGraphics);
	m_pShader->Load("Resources/Shaders/Imgui.hlsl");
	m_pVertexShader = m_pShader->GetVariation(ShaderType::VertexShader, {});
	m_pPixelShader = m_pShader->GetVariation(ShaderType::PixelShader, {});
}