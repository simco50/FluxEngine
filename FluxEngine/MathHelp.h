#pragma once

inline float randF(float min, float max)
{
	float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	float diff = max - min;
	float r = random * diff;
	return min + r;
}

inline int randI(int min, int max)
{
	return min + rand() % (max - min + 1);
}
template<typename T>
inline void Clamp(T& value, T hi, T lo)
{
	if (value > hi)
		value = hi;

	if (value < lo)
		value = lo;
}