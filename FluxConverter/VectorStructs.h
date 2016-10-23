#pragma once
struct Vector4
{
	Vector4() :x(0), y(0), z(0), w(0) {}
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	float x, y, z, w;
};
struct Vector3
{
	Vector3() :x(0), y(0), z(0) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	float x, y, z;
};
struct Vector2
{
	Vector2() : x(0), y(0) {}
	Vector2(float x, float y) : x(x), y(y) {}
	float x, y;
};

struct Matrix3x3
{
	Matrix3x3(){}
	Matrix3x3(Vector3 r1, Vector3 r2, Vector3 r3):r1(r1), r2(r2), r3(r3){}
	Vector3 r1, r2, r3;
};