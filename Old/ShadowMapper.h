#pragma once
class ShadowMapMaterial;
class GameObject;

class ShadowMapper
{
public:
	ShadowMapper();
	~ShadowMapper();

	void Initialize(GameContext* pGameContext);
	void Begin();
	void End();

	void SetLight(const XMFLOAT3 &position, const XMFLOAT3& direction);
	void Render(GameObject* pGameObject);
	ID3D11ShaderResourceView* GetShadowMap() const;
	XMFLOAT4X4 GetLightVP() const { return m_ViewProjectionMatrix; }
	XMFLOAT3 GetLightDirection() const { return m_LightDirection; }

	void SetResolution(const int width, const int height);

private:
	GameContext* m_pGameContext = nullptr;

	unique_ptr<RenderTarget> m_pRenderTarget;
	XMFLOAT4X4 m_WorldMatrix;
	XMFLOAT4X4 m_ViewProjectionMatrix;

	int m_ShadowMapWidth = 2048;
	int m_ShadowMapHeight = 2048;
	int m_Size = 50;
	D3D11_VIEWPORT m_Viewport;

	unique_ptr<ShadowMapMaterial> m_pShadowMapMaterial;
	XMFLOAT3 m_LightDirection;
};

