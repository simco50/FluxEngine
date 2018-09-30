#pragma once

struct Math
{
	template<typename T>
	inline static T Max(const T& a, const T& b)
	{
		return a < b ? b : a;
	}

	template<typename T>
	inline static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	inline static T Max3(const T& a, const T& b, const T& c)
	{
		if (a < b)
			return b < c ? c : b;
		return a < c ? c : b;
	}

	template<typename T>
	inline static T Min3(const T& a, const T& b, const T& c)
	{
		if (a < b)
			return a < c ? a : c;
		return b < c ? b : c;
	}

	inline static float RandomRange(float min, float max)
	{
		float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		float diff = max - min;
		float r = random * diff;
		return min + r;
	}

	inline static int RandomRange(int min, int max)
	{
		return min + rand() % (max - min + 1);
	}

	template<typename T>
	inline static T Clamp(const T value, const T hi, const T lo)
	{
		if (value > hi)
			return hi;
		else if (value < lo)
			return lo;
		return value;
	}

	template<typename T>
	inline static void ClampMin(T& value, const T lo)
	{
		if (value < lo)
			value = lo;
	}

	template<typename T>
	inline static T ClampMin(const T value, const T lo)
	{
		return value < lo ? lo : value;
	}

	template<typename T>
	inline static void ClampMax(T& value, const T hi)
	{
		if (value > hi)
			value = hi;
	}

	template<typename T>
	inline static T ClampMax(const T value, const T hi)
	{
		return value > hi ? hi : value;
	}

	template<typename T>
	inline static T Average(const T& a, const T& b)
	{
		return (a + b) / (T)2;
	}

	template<typename T>
	inline static T Average3(const T& a, const T& b, const T& c)
	{
		return (a + b + c) / (T)3;
	}

	template<typename T>
	inline static void Clamp01(T& value)
	{
		if (value > 1)
			value = 1;
		else if (value < 0)
			value = 0;
	}

	template<typename T>
	inline static T Clamp01(const T value)
	{
		if (value > 1)
			return 1;
		else if (value < 0)
			return 0;
		return value;
	}

	inline static float RadToDeg(float rad)
	{
		return rad * 57.2957795131f;
	}

	inline static float DegToRad(float deg)
	{
		return deg * 0.01745329251f;
	}

	inline static float Lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}

	inline static float InverseLerp(float a, float b, float value)
	{
		return (value - a) / (b - a);
	}

	inline static Vector3 ScaleFromMatrix(const Matrix& m)
	{
		return Vector3(
			sqrtf(m._11 * m._11 + m._21 * m._21 + m._31 * m._31),
			sqrtf(m._12 * m._12 + m._22 * m._22 + m._32 * m._32),
			sqrtf(m._13 * m._13 + m._23 * m._23 + m._33 * m._33));
	}

	inline static Quaternion LookRotation(const Vector3& direction)
	{
		Vector3 v;
		direction.Normalize(v);
		float pitch = asin(-v.y);
		float yaw = atan2(v.x, v.z);
		return Quaternion::CreateFromYawPitchRoll(yaw, pitch, 0);
	}

	inline static std::string ToBinary(unsigned int number)
	{
		return Math::ToBase(number, 2);
	}

	inline static std::string ToHex(unsigned int number)
	{
		return Math::ToBase(number, 16);
	}

	inline static std::string ToBase(unsigned int number, unsigned int base)
	{
		std::stringstream nr;
		unsigned int count = 0;
		while (number != 0)
		{
			unsigned int mod = number % base;
			if (mod > 9)
			{
				nr << (char)('A' + mod - 10);
			}
			else
			{
				nr << mod;
			}
			number /= base;
			++count;
		}
		for (count; count <= 8; ++count)
		{
			nr << '0';
		}
		if (base == 2)
		{
			nr << "b0";
		}
		else if (base == 8)
		{
			nr << "c0";
		}
		else if (base == 16)
		{
			nr << "x0";
		}
		std::string out = nr.str();
		std::reverse(out.begin(), out.end());
		return out;
	}

	static Vector3 RandVector()
	{
		Matrix randomMatrix = XMMatrixRotationRollPitchYaw(Math::RandomRange(-XM_PI, XM_PI), Math::RandomRange(-XM_PI, XM_PI), Math::RandomRange(-XM_PI, XM_PI));
		return Vector3::Transform(Vector3(1, 0, 0), randomMatrix);
	}

	static Vector3 RandCircleVector()
	{
		Vector3 output;
		output.z = 0;
		output.y = cos(RandomRange(-XM_PI, XM_PI));
		output.x = sin(RandomRange(-XM_PI, XM_PI));
		return output;
	}

	using HexColor = unsigned int;
	//Helper class to easily convert between 4 float colors and unsigned int hex colors
	struct HexColorConverter
	{
		Color operator()(HexColor color) const
		{
			Color output;
			//unsigned int layout: AAAA RRRR GGGG BBBB
			output.x = (float)((color >> 16) & 0xFF) / 255.0f;
			output.y = (float)((color >> 8) & 0xFF) / 255.0f;
			output.z = (float)(color & 0xFF) / 255.0f;
			output.w = (float)((color >> 24) & 0xFF) / 255.0f;
			return output;
		}

		HexColor operator()(const Color& color) const
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
};