#pragma once
#include "Core\Subsystem.h"

class Graphics;
class VertexBuffer;
class IndexBuffer;
class Shader;
class ShaderVariation;
struct VertexElement;
class ConstantBuffer;
class Texture;
class InputEngine;
class Window;

class ImmediateUI : public Subsystem
{
	FLUX_OBJECT(ImmediateUI, Subsystem)

public:
	ImmediateUI(Graphics* pGraphics, Window* pWindow, InputEngine* pInput);
	~ImmediateUI();

	DELETE_COPY(ImmediateUI)

	void NewFrame();
	void Render();
	void WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	Graphics* m_pGraphics;
	Window* m_pWindow;
	InputEngine* m_pInput;

	DelegateHandle m_WndProcHandle;

	vector<VertexElement> m_VertexElements;

	unique_ptr<Texture> m_pFontTexture;
	unique_ptr<VertexBuffer> m_pVertexBuffer;
	unique_ptr<IndexBuffer> m_pIndexBuffer;
	unique_ptr<Shader> m_pShader;
	ShaderVariation* m_pPixelShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;

	static const int START_VERTEX_COUNT = 1000;
	static const int START_INDEX_COUNT = 1000;
};