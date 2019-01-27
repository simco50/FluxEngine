#include "FluxEngine.h"
#include "MathHelp.h"

namespace Math
{

	float RandomRange(float min, float max)
	{
		float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		float diff = max - min;
		float r = random * diff;
		return min + r;
	}

	int RandomRange(int min, int max)
	{
		return min + rand() % (max - min + 1);
	}

	float Lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}

	float InverseLerp(float a, float b, float value)
	{
		return (value - a) / (b - a);
	}

	Matrix CreatePerspectiveMatrix(float FoV, float aspectRatio, float nearPlane, float farPlane)
	{
#ifdef WORLD_RIGHT_HANDED
		return DirectX::XMMatrixPerspectiveFovRH(FoV, aspectRatio, nearPlane, farPlane);
#else
		return DirectX::XMMatrixPerspectiveFovLH(FoV, aspectRatio, nearPlane, farPlane);
#endif
	}

	Matrix CreatePerspectiveOffCenterMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
#ifdef WORLD_RIGHT_HANDED
		return DirectX::XMMatrixPerspectiveOffCenterRH(left, right, bottom, top, nearPlane, farPlane);
#else
		return DirectX::XMMatrixPerspectiveOffCenterLH(left, right, bottom, top, nearPlane, farPlane);
#endif
	}

	Matrix CreateOrthographicMatrix(float width, float height, float nearPlane, float farPlane)
	{
#ifdef WORLD_RIGHT_HANDED
		return DirectX::XMMatrixOrthographicRH(width, height, nearPlane, farPlane);
#else
		return DirectX::XMMatrixOrthographicLH(width, height, nearPlane, farPlane);
#endif
	}

	Matrix CreateOrthographicOffCenterMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
#ifdef WORLD_RIGHT_HANDED
		return DirectX::XMMatrixOrthographicOffCenterRH(left, right, bottom, top, nearPlane, farPlane);
#else
		return DirectX::XMMatrixOrthographicOffCenterLH(left, right, bottom, top, nearPlane, farPlane);
#endif
	}

	DirectX::SimpleMath::Vector3 ScaleFromMatrix(const Matrix& m)
	{
		return Vector3(
			sqrtf(m._11 * m._11 + m._21 * m._21 + m._31 * m._31),
			sqrtf(m._12 * m._12 + m._22 * m._22 + m._32 * m._32),
			sqrtf(m._13 * m._13 + m._23 * m._23 + m._33 * m._33));
	}

	DirectX::SimpleMath::Quaternion LookRotation(const Vector3& direction)
	{
		Vector3 v;
		direction.Normalize(v);
		float pitch = asin(-v.y);
		float yaw = atan2(v.x, v.z);
		return Quaternion::CreateFromYawPitchRoll(yaw, pitch, 0);
	}

	std::string ToBase(unsigned int number, unsigned int base)
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

	std::string ToBinary(unsigned int number)
	{
		return Math::ToBase(number, 2);
	}

	std::string ToHex(unsigned int number)
	{
		return Math::ToBase(number, 16);
	}

	DirectX::SimpleMath::Vector3 RandVector()
	{
		Matrix randomMatrix = DirectX::XMMatrixRotationRollPitchYaw(Math::RandomRange(-PI, PI), Math::RandomRange(-PI, PI), Math::RandomRange(-PI, PI));
		return Vector3::Transform(Vector3(1, 0, 0), randomMatrix);
	}

	DirectX::SimpleMath::Vector3 RandCircleVector()
	{
		Vector3 output;
		output.z = 0;
		output.y = cos(RandomRange(-PI, PI));
		output.x = sin(RandomRange(-PI, PI));
		return output;
	}

}