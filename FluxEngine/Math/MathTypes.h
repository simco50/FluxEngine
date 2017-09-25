#pragma once

struct FloatRect
{
	FloatRect() :
		Left(0), Top(0), Right(0), Bottom(0)
	{}

	FloatRect(const float left, const float top, const float right, const float bottom) :
		Left(left), Top(top), Right(right), Bottom(bottom)
	{}

	float Left;
	float Top;
	float Right;
	float Bottom;

	float GetWidth() const { return Right - Left; }
	float GetHeight() const { return Bottom - Top; }

	bool operator==(const FloatRect& other)
	{
		return Left == other.Left && Top == other.Top && Right == other.Right && Bottom == other.Bottom;
	}

	bool operator!=(const FloatRect& other)
	{
		return Left != other.Left || Top != other.Top || Right != other.Right || Bottom != other.Bottom;
	}
};

struct IntRect
{
	IntRect() :
		Left(0), Top(0), Right(0), Bottom(0)
	{}

	IntRect(const int left, const int top, const int right, const int bottom) :
		Left(left), Top(top), Right(right), Bottom(bottom)
	{}

	int Left;
	int Top;
	int Right;
	int Bottom;

	int GetWidth() const { return Right - Left; }
	int GetHeight() const { return Bottom - Top; }

	bool operator==(const IntRect& other)
	{
		return Left == other.Left && Top == other.Top && Right == other.Right && Bottom == other.Bottom;
	}

	bool operator!=(const IntRect& other)
	{
		return Left != other.Left || Top != other.Top || Right != other.Right || Bottom != other.Bottom;
	}
};