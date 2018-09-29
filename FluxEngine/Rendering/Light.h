#pragma once
#include "Scenegraph/Component.h"

class Texture;

class Light : public Component
{
	FLUX_OBJECT(Light, Component);

public:
	Light(Context* pContext);
	~Light();

	enum class Type : unsigned int
	{
		Directional = 0,
		Point = 1,
		Spot = 2,
		MAX
	};

#pragma pack(16)
	struct Data
	{
		int Enabled = true;
		Vector3 Position;
		Vector3 Direction;
		Color Colour = Color(1, 1, 1, 1);
		float Intensity = 1.0f;
		float Range = 1.0f;
		float SpotLightAngle = 45.0f;
		float Attenuation = 0.5f;
		Type Type = Light::Type::Directional;
	};

	void SetEnabled(const bool enabled) { m_Data.Enabled = (int)enabled; }
	void SetType(const Type type);
	void SetRange(const float range) { m_Data.Range = range; }
	void SetIntesity(const float intensity) { m_Data.Intensity = intensity; }
	void SetColor(const Color& color) { m_Data.Colour = color; }
	void SetShadowCasting(const bool enabled);

	constexpr static size_t GetDataStride() { return sizeof(Data); }
	Data* GetData() { return &m_Data; }
	const Data* GetData() const { return &m_Data; }

	virtual void OnSceneSet(Scene* pScene) override;
	virtual void OnSceneRemoved() override;
	virtual void OnMarkedDirty(const Transform* transform) override;

	const Matrix& GetViewProjection() const { return m_ViewProjection; }
	const Texture* GetShadowTexture() const { return m_pShadowTexture.get(); }

private:
	Matrix m_ViewProjection;
	std::unique_ptr<Texture> m_pShadowTexture;

	bool m_CastShadow = false;
	Data m_Data;
};