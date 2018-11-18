#pragma once

#ifdef MODULE_D3D11
#include "External/SimpleMath/SimpleMath.h"
using BoundingBox = DirectX::BoundingBox;
using BoundingFrustum = DirectX::BoundingFrustum;
using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;
using Matrix = DirectX::SimpleMath::Matrix;
using Quaternion = DirectX::SimpleMath::Quaternion;
using Color = DirectX::SimpleMath::Color;
using Ray = DirectX::SimpleMath::Ray;
#endif

template<typename T>
struct RectT
{
	RectT()
		: Left(T()), Top(T()), Right(T()), Bottom(T())
	{}

	RectT(const T left, const T top, const T right, const T bottom)
		: Left(left), Top(top), Right(right), Bottom(bottom)
	{}

	template<typename U>
	RectT(const RectT<U>& other)
		: Left((T)other.Left), Top((T)other.Top), Right((T)other.Right), Bottom((T)other.Bottom)
	{

	}

	T Left;
	T Top;
	T Right;
	T Bottom;

	T GetWidth() const { return Right - Left; }
	T GetHeight() const { return Bottom - Top; }

	bool operator==(const RectT& other) const
	{
		return Left == other.Left && Top == other.Top && Right == other.Right && Bottom == other.Bottom;
	}

	bool operator!=(const RectT& other) const
	{
		return Left != other.Left || Top != other.Top || Right != other.Right || Bottom != other.Bottom;
	}

	static RectT ZERO()
	{
		return RectT();
	}
};

using FloatRect = RectT<float>;
using IntRect = RectT<int>;