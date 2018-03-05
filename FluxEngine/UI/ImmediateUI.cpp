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

ImmediateUI::ImmediateUI(Context* pContext) :
	Subsystem(pContext)
{
	AUTOPROFILE(ImmediateUI_Initialize);

	m_pInput = pContext->GetSubsystem<InputEngine>();
	m_pGraphics = pContext->GetSubsystem<Graphics>();
	m_SDLEventHandle = m_pInput->OnHandleSDL().AddRaw(this, &ImmediateUI::HandleSDLEvent);

	//Set ImGui parameters
	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
	io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
	io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
	io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
	io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
	io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
	io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
	io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
	io.RenderDrawListsFn = nullptr;
	io.ImeWindowHandle = m_pGraphics->GetWindow();

	//Load shader
	m_pVertexShader = m_pGraphics->GetShader("Resources/Shaders/Imgui.hlsl", ShaderType::VertexShader);
	m_pPixelShader = m_pGraphics->GetShader("Resources/Shaders/Imgui.hlsl", ShaderType::PixelShader);

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

	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	m_pFontTexture = std::make_unique<Texture>(m_pContext);
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
	
	int posX, posY;
	SDL_GetMouseState(&posX, &posY);
	io.MousePos.x = (float)posX;
	io.MousePos.y = (float)posY;
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
	m_pGraphics->SetShader(ShaderType::GeometryShader, nullptr);

	m_pGraphics->SetIndexBuffer(m_pIndexBuffer.get());
	m_pGraphics->SetVertexBuffer(m_pVertexBuffer.get());

	m_pGraphics->GetDepthStencilState()->SetDepthEnabled(true);
	m_pGraphics->GetDepthStencilState()->SetDepthTest(CompareMode::ALWAYS);

	m_pGraphics->GetBlendState()->SetColorWrite(ColorWrite::ALL);
	m_pGraphics->GetBlendState()->SetBlendMode(BlendMode::ALPHA, false);

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