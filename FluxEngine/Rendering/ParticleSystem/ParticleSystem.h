#pragma once
#include "Math/KeyframeValue.h"
#include "Content/Resource.h"
#include "Rendering/Core/GraphicsDefines.h"

struct ParticleVertex
{
	ParticleVertex(Vector3 pos = Vector3(), Vector4 col = (Vector4)Colors::White, float size = 5.0f, float rotation = 0) :
		Position(pos),
		Color(col),
		Size(size),
		Rotation(rotation) {}

	Vector3 Position;
	Vector4 Color;
	float Size;
	float Rotation;
};

enum class ParticleSortingMode
{
	FrontToBack,
	BackToFront,
	OldestFirst,
	YoungestFirst,
};

class ParticleSystem : public Resource
{
public:

	ParticleSystem() {}
	virtual ~ParticleSystem() {}

	virtual bool Load(const std::string& filePath) override;

	//General
	float Duration = 1.0f;
	bool Loop = true;
	float Lifetime = 1.0f;
	float LifetimeVariance = 0.0f;
	float StartVelocity = 1.0f;
	float StartVelocityVariance = 0.0f;
	float StartSize = 1.0f;
	float StartSizeVariance = 0.0f;
	bool RandomStartRotation = true;
	bool PlayOnAwake = true;
	int MaxParticles = 100;

	//Emission
	int Emission = 10;
	std::map<float, int> Bursts;

	//Shape
	enum class ShapeType
	{
		CIRCLE,
		SPHERE,
		CONE,
		EDGE,
	};
	struct Shape
	{
		ShapeType ShapeType = ShapeType::SPHERE;
		float Radius = 1.0f;
		bool EmitFromShell = false;
		bool EmitFromVolume = false;
		float Angle = 30.0f;
	};
	Shape Shape = {};

	//Animation
	KeyframeValue<float> Size = 1.0f;
	KeyframeValue<Vector3> Velocity = Vector3();
	KeyframeValue<Vector3> LocalVelocity = Vector3();
	KeyframeValue<Vector3> Color = Vector3(1.0f, 1.0f, 1.0f);
	KeyframeValue<float> Transparancy = 1.0f;
	KeyframeValue<float> Rotation = 0.0f;

	//Rendering
	ParticleSortingMode SortingMode = ParticleSortingMode::FrontToBack;
	BlendMode BlendingMode = BlendMode::ALPHA;
	std::string ImagePath = "";

	static const int VERSION = 2;
};