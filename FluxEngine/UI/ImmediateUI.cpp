#include "stdafx.h"
#include "ImmediateUI.h"
#include "Rendering\Core\Graphics.h"
#include "Rendering\Core\VertexBuffer.h"
#include "Rendering\Core\IndexBuffer.h"
#include "Rendering\Core\Shader.h"
#include "Rendering\Core\ShaderVariation.h"
#include "Rendering\Core\ConstantBuffer.h"
#include "Rendering\Core\Texture.h"
#include "Rendering\Core\DepthStencilState.h"
#include "Rendering\Core\BlendState.h"
#include "Rendering\Core\RasterizerState.h"

ImmediateUI::ImmediateUI(Graphics* pGraphics, InputEngine* pInput) :
	m_pGraphics(pGraphics),
	m_pInput(pInput)
{
	//Set ImGui parameters
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

	//Load shader
	m_pShader = make_unique< Shader>(m_pGraphics);
	m_pShader->Load("Resources/Shaders/Imgui.hlsl");
	m_pVertexShader = m_pShader->GetVariation(ShaderType::VertexShader, {});
	m_pPixelShader = m_pShader->GetVariation(ShaderType::PixelShader, {});

	//Create vertex buffer
	m_pVertexBuffer.reset();
	m_pVertexBuffer = make_unique<VertexBuffer>(m_pGraphics);
	m_VertexElements.push_back(VertexElement(VertexElementType::VECTOR2, VertexElementSemantic::POSITION));
	m_VertexElements.push_back(VertexElement(VertexElementType::VECTOR2, VertexElementSemantic::TEXCOORD));
	m_VertexElements.push_back(VertexElement(VertexElementType::UBYTE4_NORM, VertexElementSemantic::COLOR));
	m_pVertexBuffer->Create(START_VERTEX_COUNT, m_VertexElements, true);

	//Create index buffer
	m_pIndexBuffer.reset();
	m_pIndexBuffer = make_unique<IndexBuffer>(m_pGraphics);
	m_pIndexBuffer->Create(START_INDEX_COUNT, true, true);

	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	m_pFontTexture = make_unique<Texture>(m_pGraphics);
	m_pFontTexture->SetSize(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::STATIC, 1, nullptr);
	m_pFontTexture->SetData(pixels);
	io.Fonts->TexID = m_pFontTexture.get();
}

ImmediateUI::~ImmediateUI()
{
	ImGui::Shutdown();
}

void ImmediateUI::NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight());
	io.DeltaTime = GameTimer::DeltaTime();
	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	io.MouseDrawCursor = true;
	m_pInput->CursorVisible(!io.MouseDrawCursor);

	ImGui::NewFrame();
}

void ImmediateUI::Render()
{
	ImGui::Render();
	ImDrawData* pDrawData = ImGui::GetDrawData();

	//Recreate the vertexbuffer if it is not large enough
	if(m_pVertexBuffer == nullptr || (int)m_pVertexBuffer->GetVertexCount() < pDrawData->TotalVtxCount)
		m_pVertexBuffer->Create(m_pVertexBuffer->GetVertexCount() + 5000, m_VertexElements, true);

	//Recreate the indexbuffer if it is not large enough
	if (m_pIndexBuffer == nullptr || (int)m_pIndexBuffer->GetCount() < pDrawData->TotalIdxCount)
		m_pIndexBuffer->Create(m_pIndexBuffer->GetCount() + 10000, true, true);

	//Copy the new data to the buffers
	ImDrawVert* pVertexData = (ImDrawVert*)m_pVertexBuffer->Map(true);
	ImDrawIdx* IndexData = (ImDrawIdx*)m_pIndexBuffer->Map(true);

	for (int n = 0; n < pDrawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = pDrawData->CmdLists[n];
		memcpy(pVertexData, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(IndexData, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		pVertexData += cmd_list->VtxBuffer.Size;
		IndexData += cmd_list->IdxBuffer.Size;
	}

	m_pVertexBuffer->Unmap();
	m_pIndexBuffer->Unmap();

	m_pGraphics->SetShader(ShaderType::VertexShader, m_pVertexShader);
	m_pGraphics->SetShader(ShaderType::PixelShader, m_pPixelShader);

	m_pGraphics->SetIndexBuffer(m_pIndexBuffer.get());
	m_pGraphics->SetVertexBuffer(m_pVertexBuffer.get());

	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::ALWAYS);

	m_pGraphics->GetBlendState()->SetColorWrite(ColorWrite::ALL);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::ALPHA, false);

	m_pGraphics->GetRasterizerState()->SetCullMode(CullMode::BACK);

	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight()));

	Matrix projectionMatrix = XMMatrixOrthographicOffCenterLH(0.0f, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight(), 0.0f, 0.0f, 1.0f);
	m_pVertexShader->SetParameter("cViewProjVS", projectionMatrix);

	int vertexOffset = 0;
	int indexOffset = 0;
	for (int n = 0; n < pDrawData->CmdListsCount; n++)
	{
		const ImDrawList* pCmdList = pDrawData->CmdLists[n];
		for (int cmd_i = 0; cmd_i < pCmdList->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &pCmdList->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
				pcmd->UserCallback(pCmdList, pcmd);
			else
			{
				m_pGraphics->SetScissorRect(true, {
					(int)pcmd->ClipRect.x, 
					(int)pcmd->ClipRect.y, 
					(int)pcmd->ClipRect.z, 
					(int)pcmd->ClipRect.w });
				m_pGraphics->SetTexture(TextureSlot::Diffuse, (Texture*)pcmd->TextureId);
				m_pGraphics->DrawIndexed(PrimitiveType::TRIANGLELIST, pcmd->ElemCount, indexOffset, vertexOffset);
			}
			indexOffset += pcmd->ElemCount;
		}
		vertexOffset += pCmdList->VtxBuffer.Size;
	}
}

int ImmediateUI::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (message)
	{
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		return 1;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return 1;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		return 1;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return 1;
	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		return 1;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		return 1;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return 1;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return 1;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return 1;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return 1;
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
	default:
		break;
	}

	return 0;
}