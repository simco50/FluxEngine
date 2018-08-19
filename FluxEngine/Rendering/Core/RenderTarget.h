#pragma once

class Texture;

class RenderTarget
{
	friend class Texture;

public:
	RenderTarget(Texture* pTexture);
	~RenderTarget();

	DELETE_COPY(RenderTarget)

	void Release();

	void* GetRenderTargetView() const { return m_pRenderTargetView; }
	Texture* GetParentTexture() const { return m_pParentTexture; }

	Texture* m_pParentTexture = nullptr;
	void* m_pRenderTargetView = nullptr;
};