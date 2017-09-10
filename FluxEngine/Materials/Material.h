#pragma once
enum class RenderQueueID
{
	ID_OPAQUE,
	ID_TRANSPARANT,
	ID_DEPTH,
};

struct MaterialDesc
{
	string EffectName = "";
	string TechniqueName = "";
	bool HasWorldMatrix = false;
	bool HasViewMatrix = false;
	bool HasViewInverseMatrix = false;
	bool HasWvpMatrix = false;
	RenderQueueID RenderQueue = RenderQueueID::ID_OPAQUE;

	bool Validate() const
	{
		if (EffectName.size() == 0)
		{
			Console::Log("Material::Initialize() > MaterialDesc is invalid!", LogType::ERROR);
			return false;
		}
		return true;
	}
};

class Material
{
public:
	Material() {}
	virtual ~Material();

	void Initialize(GameContext* pGameContext);
	bool IsInitialized() const { return m_IsInitialized; }

	void LoadEffect();
	void Update(const XMFLOAT4X4& WorldMatrix);

	ID3D11InputLayout* GetInputLayout() const { return m_pInputLayout.Get(); }
	const InputLayoutDesc* GetInputLayoutDesc() const { return &m_InputLayoutDesc; }
	const MaterialDesc& GetDesc() const { return m_MaterialDesc; }
	virtual ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }

protected:
	GameContext* m_pGameContext = nullptr;

	virtual void LoadShaderVariables() = 0;
	virtual void UpdateShaderVariables() = 0;
	virtual void CreateInputLayout();

	MaterialDesc m_MaterialDesc;
	ID3DX11Effect* m_pEffect = nullptr;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;
	Unique_COM<ID3D11InputLayout> m_pInputLayout;

private:
	InputLayoutDesc m_InputLayoutDesc;

	//Effect variables
	ID3DX11EffectMatrixVariable *m_pWorldMatrixVariable = nullptr;
	ID3DX11EffectMatrixVariable *m_pViewMatrixVariable = nullptr;
	ID3DX11EffectMatrixVariable *m_pViewInverseMatrixVariable = nullptr;
	ID3DX11EffectMatrixVariable *m_pWvpMatrixVariable = nullptr;

	bool m_IsInitialized = false;
};

