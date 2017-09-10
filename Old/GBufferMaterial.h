#pragma once
#include "../Materials/Material.h"

class GBufferMaterial : public Material
{
public:
	GBufferMaterial();
	~GBufferMaterial();

	void SetDiffuseSRV(ID3D11ShaderResourceView* pSRV);
	void SetNormalSRV(ID3D11ShaderResourceView* pSRV);
	void SetDepthSRV(ID3D11ShaderResourceView* pSRV);
	void SetEyePos(const XMFLOAT3& eyePos);
	void SetViewProjInv(const XMFLOAT4X4& viewProjInv);


	void SetLightVP(const XMFLOAT4X4& vp);
	void SetLightDirection(const XMFLOAT3& direction);
	void SetShadowMap(ID3D11ShaderResourceView* pSRV);

private:

	void LoadShaderVariables() override;
	void UpdateShaderVariables() override;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseSRV = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pNormalSRV = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pDepthSRV = nullptr;
	ID3DX11EffectVectorVariable* m_pEyePosVar = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewProjInvVar = nullptr;

	ID3DX11EffectMatrixVariable* m_pLightVP = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pShadowMap = nullptr;
	ID3DX11EffectVectorVariable* m_pLightDirection = nullptr;
};

