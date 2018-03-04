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

class ImmediateUI : public Subsystem
{
	FLUX_OBJECT(ImmediateUI, Subsystem)

public:
	ImmediateUI(Context* pContext);
	~ImmediateUI();

	DELETE_COPY(ImmediateUI)

	void NewFrame();
	void Render();

private:
	void HandleSDLEvent(SDL_Event* pEvent);

	Graphics* m_pGraphics;
	InputEngine* m_pInput;

	DelegateHandle m_SDLEventHandle;

	std::vector<VertexElement> m_VertexElements;

	std::unique_ptr<Texture> m_pFontTexture;
	std::unique_ptr<VertexBuffer> m_pVertexBuffer;
	std::unique_ptr<IndexBuffer> m_pIndexBuffer;
	ShaderVariation* m_pPixelShader = nullptr;
	ShaderVariation* m_pVertexShader = nullptr;

	static const int START_VERTEX_COUNT = 1000;
	static const int START_INDEX_COUNT = 1000;
};