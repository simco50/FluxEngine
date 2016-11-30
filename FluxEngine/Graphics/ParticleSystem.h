#pragma once
#include "../Helpers/KeyframeValue.h"

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

enum ParticleSortingMode
{
	OldestFirst,
	YoungestFirst,
	ByDistance,
};

struct ParticleSystem
{
	ParticleSystem() :
		Lifetime(1.0f),
		LifetimeVariance(0.0f),
		StartVelocity(0.0f),
		StartVelocityVariance(0.0f),
		Size(1.0f),
		Velocity(Vector3()),
		LocalVelocity(Vector3()),
		Color(Vector3(1, 1, 1)),
		Transparany(1),
		Rotation(0.0f)
	{}
	~ParticleSystem()
	{}

	float Lifetime;
	float LifetimeVariance;
	int Emission = 10;
	int MaxParticles = 100;
	float StartVelocity;
	float StartVelocityVariance;

	KeyframeValue<float> Size;
	KeyframeValue<Vector3> Velocity;
	KeyframeValue<Vector3> LocalVelocity;
	KeyframeValue<Vector3> Color;
	KeyframeValue<float> Transparany;
	KeyframeValue<float> Rotation;
	bool RandomStartRotation = true;

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
	Shape Shape;

	ParticleSortingMode SortingMode = ByDistance;
	wstring ImagePath = L"";
};