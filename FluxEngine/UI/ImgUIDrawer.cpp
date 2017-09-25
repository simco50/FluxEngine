#include "stdafx.h"
#include "ImgUIDrawer.h"

void ImgUIDrawer::Initialize(const HWND& hwnd)
{
	m_Hwnd = hwnd;

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

	io.ImeWindowHandle = hwnd;

	LoadShader();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateFontsTexture();
}

void ImgUIDrawer::Shutdown()
{
	ImGui::Shutdown();
}

void ImgUIDrawer::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();

	RECT rect;
	GetClientRect(m_Hwnd, &rect);
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

	if(m_pVertexBuffer.Get() == nullptr || m_VertexBufferSize < draw_data->TotalVtxCount)
	{
		m_VertexBufferSize = draw_data->TotalVtxCount + 5000;
		CreateVertexBuffer();
	}
	
	if (m_pIndexBuffer.Get() == nullptr || m_IndexBufferSize < draw_data->TotalIdxCount)
	{
		m_IndexBufferSize = draw_data->TotalIdxCount + 10000;
		CreateIndexBuffer();
	}

	D3D11_MAPPED_SUBRESOURCE indexResource, vertexResource;
	HR(RenderSystem::Instance().GetDeviceContext()->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vertexResource))
	HR(RenderSystem::Instance().GetDeviceContext()->Map(m_pIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &indexResource))

	ImDrawVert* vtx_dst = (ImDrawVert*)vertexResource.pData;
	ImDrawIdx* idx_dst = (ImDrawIdx*)indexResource.pData;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}
	RenderSystem::Instance().GetDeviceContext()->Unmap(m_pVertexBuffer.Get(), 0);
	RenderSystem::Instance().GetDeviceContext()->Unmap(m_pIndexBuffer.Get(), 0);

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
	m_pViewProjVariable->SetMatrix((const float*)mvp[0]);

	//Render
	RenderSystem::Instance().SetInputLayout(m_pInputLayout.Get());
	RenderSystem::Instance().SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	RenderSystem::Instance().SetIndexBuffer(m_pIndexBuffer.Get(), sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
	UINT offset = 0;
	UINT stride = sizeof(ImDrawVert);
	RenderSystem::Instance().SetVertexBuffer(0, m_pVertexBuffer.Get(), &stride, &offset);


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
				const D3D11_RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
				m_pTextureVariable->SetResource((ID3D11ShaderResourceView*)pcmd->TextureId);
				RenderSystem::Instance().GetDeviceContext()->RSSetScissorRects(1, &r);
				m_pTechnique->GetPassByIndex(0)->Apply(0, RenderSystem::Instance().GetDeviceContext());
				RenderSystem::Instance().GetDeviceContext()->DrawIndexed(pcmd->ElemCount, indexOffset, vertexOffset);
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
	m_pVertexBuffer.Reset();

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = m_VertexBufferSize * sizeof(ImDrawVert);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	HR(RenderSystem::Instance().GetDevice()->CreateBuffer(&desc, nullptr, m_pVertexBuffer.GetAddressOf()))
}

void ImgUIDrawer::CreateIndexBuffer()
{
	m_pIndexBuffer.Reset();

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.ByteWidth = m_IndexBufferSize * sizeof(ImDrawIdx);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	HR(RenderSystem::Instance().GetDevice()->CreateBuffer(&desc, nullptr, m_pIndexBuffer.GetAddressOf()))
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
	HR(RenderSystem::Instance().GetDevice()->CreateTexture2D(&desc, &subResource, &pTexture))

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(RenderSystem::Instance().GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, m_pFontSRV.GetAddressOf()))
	pTexture->Release();

	io.Fonts->TexID = (void*)m_pFontSRV.Get();
}

void ImgUIDrawer::LoadShader()
{
	m_pEffect = ResourceManager::Load<ID3DX11Effect>("Resources/Shaders/ImGui.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	D3D11_INPUT_ELEMENT_DESC elementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	D3DX11_PASS_DESC passDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(RenderSystem::Instance().GetDevice()->CreateInputLayout(elementDesc, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, m_pInputLayout.GetAddressOf()))

	BIND_AND_CHECK_NAME(m_pTextureVariable, gTexture, AsShaderResource);
	BIND_AND_CHECK_NAME(m_pViewProjVariable, gViewProj, AsMatrix);
}