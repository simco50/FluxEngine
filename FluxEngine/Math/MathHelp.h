#pragma once

inline float RandF(float min, float max)
{
	float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	float diff = max - min;
	float r = random * diff;
	return min + r;
}

inline int RandI(int min, int max)
{
	return min + rand() % (max - min + 1);
}
template<typename T>
inline void Clamp(T& value, T hi, T lo)
{
	if (value > hi)
		value = hi;

	else if (value < lo)
		value = lo;
}

template<typename T>
inline void ClampMin(T& value, T lo)
{
	if (value < lo)
		value = lo;
}

template<typename T>
inline void ClampMax(T& value, T hi)
{
	if (value > hi)
		value = hi;
}

template<typename T>
inline void Clamp01(T& value)
{
	if (value > 1)
		value = 1;
	else if (value < 0)
		value = 0;
}

inline float RadToDeg(float rad)
{
	return rad * 57.2957795131f;
}

inline float DegToRad(float deg)
{
	return deg * 0.01745329251f;
}

inline float Lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

inline float InverseLerp(float a, float b, float value)
{
	return (value - a) / (b - a);
}

inline std::string ToBinary(unsigned int number)
{
	std::stringstream nr;
	while (number != 0)
	{ 
		nr << number % 2;
		number /= 2;
	}
	nr << "b0";
	std::string out = nr.str();
	std::reverse(out.begin(), out.end());
	return out;
}

inline std::string ToHex(unsigned int number)
{
	std::stringstream nr;
	while (number != 0)
	{
		nr << number % 16;
		number /= 16;
	}
	nr << "x0";
	std::string out = nr.str();
	std::reverse(out.begin(), out.end());
	return out;
}

using HexColor = unsigned int;
//Helper class to easily convert between 4 float colors and unsigned int hex colors
struct HexColorConverter
{
	Color operator()(HexColor color)
	{
		Color output;
		//unsigned int layout: AAAA RRRR GGGG BBBB
		output.x = (float)((color >> 16) & 0xFF) / 255.0f;
		output.y = (float)((color >> 8) & 0xFF) / 255.0f;
		output.z = (float)(color & 0xFF) / 255.0f;
		output.w = (float)((color >> 24) & 0xFF) / 255.0f;
		return output;
	}

	HexColor operator()(const Color& color)
	{
		HexColor output = 0;
		//unsigned int layout: AAAA RRRR GGGG BBBB
		output |= (unsigned char)(color.x * 255.0f) << 16;
		output |= (unsigned char)(color.y * 255.0f) << 8;
		output |= (unsigned char)(color.z * 255.0f);
		output |= (unsigned char)(color.w * 255.0f) << 24;
		return output;
	}
};