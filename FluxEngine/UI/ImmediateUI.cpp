#include "FluxEngine.h"
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
#include "Input/InputEngine.h"
#include "Rendering\Core\Texture2D.h"

ImmediateUI::ImmediateUI(Context* pContext) :
	Subsystem(pContext)
{
	AUTOPROFILE(ImmediateUI_Initialize);

	ImGui::CreateContext();

	m_pInput = pContext->GetSubsystem<InputEngine>();
	m_pGraphics = pContext->GetSubsystem<Graphics>();
	m_SDLEventHandle = m_pInput->OnHandleSDL().AddRaw(this, &ImmediateUI::HandleSDLEvent);

	//Set ImGui parameters
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = (int)KeyboardKey::KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = (int)KeyboardKey::KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = (int)KeyboardKey::KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = (int)KeyboardKey::KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = (int)KeyboardKey::KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = (int)KeyboardKey::KEY_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = (int)KeyboardKey::KEY_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = (int)KeyboardKey::KEY_HOME;
	io.KeyMap[ImGuiKey_End] = (int)KeyboardKey::KEY_END;
	io.KeyMap[ImGuiKey_Delete] = (int)KeyboardKey::KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = (int)KeyboardKey::KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter] = (int)KeyboardKey::KEY_RETURN;
	io.KeyMap[ImGuiKey_Escape] = (int)KeyboardKey::KEY_ESCAPE;
	io.KeyMap[ImGuiKey_Space] = (int)KeyboardKey::KEY_SPACE;
	io.KeyMap[ImGuiKey_Insert] = (int)KeyboardKey::KEY_INSERT;
	io.KeyMap[ImGuiKey_A] = (int)KeyboardKey::KEY_A;
	io.KeyMap[ImGuiKey_C] = (int)KeyboardKey::KEY_C;
	io.KeyMap[ImGuiKey_V] = (int)KeyboardKey::KEY_V;
	io.KeyMap[ImGuiKey_X] = (int)KeyboardKey::KEY_X;
	io.KeyMap[ImGuiKey_Y] = (int)KeyboardKey::KEY_Y;
	io.KeyMap[ImGuiKey_Z] = (int)KeyboardKey::KEY_Z;
	io.RenderDrawListsFn = nullptr;
	io.ImeWindowHandle = m_pGraphics->GetWindow();

	SetStyle(true, 0.9f);

	//Load shader
	m_pVertexShader = m_pGraphics->GetShader("Resources/Shaders/Imgui", ShaderType::VertexShader);
	m_pPixelShader = m_pGraphics->GetShader("Resources/Shaders/Imgui", ShaderType::PixelShader);

	//Create vertex buffer
	m_pVertexBuffer.reset();
	m_pVertexBuffer = std::make_unique<VertexBuffer>(m_pGraphics);
	m_VertexElements.push_back(VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::POSITION));
	m_VertexElements.push_back(VertexElement(VertexElementType::FLOAT2, VertexElementSemantic::TEXCOORD));
	m_VertexElements.push_back(VertexElement(VertexElementType::UBYTE4_NORM, VertexElementSemantic::COLOR));
	m_pVertexBuffer->Create(START_VERTEX_COUNT, m_VertexElements, true);

	//Create index buffer
	m_pIndexBuffer.reset();
	m_pIndexBuffer = std::make_unique<IndexBuffer>(m_pGraphics);
	m_pIndexBuffer->Create(START_INDEX_COUNT, true, true);

	const char fontPath[] = "Resources/OpenSans-Regular.ttf";
	std::unique_ptr<File> pFile = FileSystem::GetFile(fontPath);
	if (pFile && pFile->OpenRead())
	{
		size_t size = pFile->GetSize();
		unsigned char* pBuffer = new unsigned char[size];
		pFile->Read(pBuffer, size);
		pFile->Close();
		io.Fonts->AddFontFromMemoryTTF(pBuffer, (int)size, 18.0f);
	}
	else
	{
		FLUX_LOG(Warning, "[ImmediadeUI::ImmediateUI() > Font not found '%s']", fontPath);
	}
	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	m_pFontTexture = std::make_unique<Texture2D>(m_pContext);
	m_pFontTexture->SetSize(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, TextureUsage::STATIC, 1, nullptr);
	m_pFontTexture->SetData(0, 0, 0, width, height, pixels);
	io.Fonts->TexID = m_pFontTexture.get();
}

ImmediateUI::~ImmediateUI()
{
	ImGui::DestroyContext();
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
	
	Vector2 mousePos = m_pInput->GetMousePosition();
	io.MousePos.x = mousePos.x;
	io.MousePos.y = mousePos.y;
	io.MouseDrawCursor = true;

	m_pInput->CursorVisible(!io.MouseDrawCursor);

	ImGui::NewFrame();
}

void ImmediateUI::Render()
{
	ImGui::Render();

	ImDrawData* pDrawData = ImGui::GetDrawData();

	//Recreate the vertexbuffer if it is not large enough
	if((int)m_pVertexBuffer->GetVertexCount() < pDrawData->TotalVtxCount)
		m_pVertexBuffer->Create(m_pVertexBuffer->GetVertexCount() + 5000, m_VertexElements, true);

	//Recreate the indexbuffer if it is not large enough
	if ((int)m_pIndexBuffer->GetCount() < pDrawData->TotalIdxCount)
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

	m_pGraphics->InvalidateShaders();
	m_pGraphics->SetShader(ShaderType::VertexShader, m_pVertexShader);
	m_pGraphics->SetShader(ShaderType::PixelShader, m_pPixelShader);

	m_pGraphics->SetIndexBuffer(m_pIndexBuffer.get());
	m_pGraphics->SetVertexBuffer(m_pVertexBuffer.get());

	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::ALWAYS);

	m_pGraphics->GetBlendState()->SetColorWrite(ColorWrite::ALL);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::ALPHA, false);

	m_pGraphics->GetRasterizerState()->SetFillMode(FillMode::SOLID);
	m_pGraphics->GetRasterizerState()->SetCullMode(CullMode::BACK);

	m_pGraphics->SetViewport(FloatRect(0.0f, 0.0f, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight()));

	Matrix projectionMatrix = XMMatrixOrthographicOffCenterLH(0.0f, (float)m_pGraphics->GetWindowWidth(), (float)m_pGraphics->GetWindowHeight(), 0.0f, 0.0f, 1.0f);
	m_pGraphics->SetShaderParameter("cViewProj", &projectionMatrix);

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
				m_pGraphics->SetTexture(TextureSlot::Diffuse, static_cast<Texture*>(pcmd->TextureId));
				m_pGraphics->DrawIndexed(PrimitiveType::TRIANGLELIST, pcmd->ElemCount, indexOffset, vertexOffset);
			}
			indexOffset += pcmd->ElemCount;
		}
		vertexOffset += pCmdList->VtxBuffer.Size;
	}
}

void ImmediateUI::HandleSDLEvent(SDL_Event* pEvent)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (pEvent->type)
	{
	case SDL_MOUSEWHEEL:
	{
		if (pEvent->wheel.y > 0) io.MouseWheel += 1;
		if (pEvent->wheel.y < 0) io.MouseWheel -= 1;
		return;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		if (pEvent->button.button == SDL_BUTTON_LEFT) io.MouseDown[0] = true;
		if (pEvent->button.button == SDL_BUTTON_RIGHT) io.MouseDown[1] = true;
		if (pEvent->button.button == SDL_BUTTON_MIDDLE) io.MouseDown[2] = true;
		return;
	}
	case SDL_MOUSEBUTTONUP:
	{
		if (pEvent->button.button == SDL_BUTTON_LEFT) io.MouseDown[0] = false;
		if (pEvent->button.button == SDL_BUTTON_RIGHT) io.MouseDown[1] = false;
		if (pEvent->button.button == SDL_BUTTON_MIDDLE) io.MouseDown[2] = false;
		return;
	}
	case SDL_TEXTINPUT:
	{
		io.AddInputCharactersUTF8(pEvent->text.text);
		return;
	}
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	{
		int key = pEvent->key.keysym.scancode;
		io.KeysDown[key] = (pEvent->type == SDL_KEYDOWN);
		io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
		io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
		io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
		io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
		return;
	}
	}
	return;
}

void ImmediateUI::SetStyle(bool dark, float alpha)
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.FrameRounding = 3.0f;
	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	if (dark)
	{
		for (int i = 0; i < ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			float H, S, V;
			ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

			if (S < 0.1f)
			{
				V = 1.0f - V;
			}
			ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
			if (col.w < 1.00f)
			{
				col.w *= alpha;
			}
		}
	}
	else
	{
		for (int i = 0; i <= ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			if (col.w < 1.00f)
			{
				col.x *= alpha;
				col.y *= alpha;
				col.z *= alpha;
				col.w *= alpha;
			}
		}
	}
}
