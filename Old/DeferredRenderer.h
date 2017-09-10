#pragma once
class RenderTarget;
class GBufferMaterial;
class QuadRenderer;

class DeferredRenderer
{
public:
	DeferredRenderer();
	~DeferredRenderer();

	void Initialize(GameContext* pGameContext);
	void CreateGBuffer();
	void Begin();
	void End();

	enum GBUFFER_ID : int
	{
		DIFFUSE = 0,
		NORMAL = 1,
	};

private:
	void ClearRenderTargets();
	void SetRenderTargets();

	static const int GBUFFER_SIZE = 2;
	vector<unique_ptr<RenderTarget>> m_pGBuffer;

	GameContext* m_pGameContext = nullptr;

	unique_ptr<GBufferMaterial> m_pMaterial;
	QuadRenderer* m_pQuadRenderer = nullptr;
};

