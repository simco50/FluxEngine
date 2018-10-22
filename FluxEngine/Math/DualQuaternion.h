#pragma once

/*
Code from "A Beginners Guide to Dual-Quaternions" converted to C++

What They Are, How They Work, and How to Use Them for 3D Character Hierarchies
Ben Kenwright
School of Computing Science, Newcastle University
Newcastle Upon Tyne, United Kingdom
b.kenwright@ncl.ac.uk
*/

struct DualQuaternion
{
	DualQuaternion()
		: Real(0, 0, 0, 1), Dual(0, 0, 0, 0)
	{}

	DualQuaternion(const Quaternion& real, const Quaternion& dual)
		: Real(real), Dual(dual)
	{}

	DualQuaternion(const DualQuaternion& other)
		: Real(other.Real), Dual(other.Dual)
	{}

	DualQuaternion& operator=(const DualQuaternion& other)
	{
		Real = other.Real;
		Dual = other.Dual;
		return *this;
	}

	DualQuaternion(const Quaternion& rotation, const Vector3& position)
	{
		rotation.Normalize(Real);
		Dual = 0.5f * Real * Quaternion(position, 0);
	}

	DualQuaternion& operator+(const DualQuaternion& other)
	{
		Real = Real + other.Real;
		Dual = Dual + other.Dual;
		return *this;
	}

	DualQuaternion& operator*(const DualQuaternion& other)
	{
		Real = Real * other.Real;
		Dual = other.Dual * Real + other.Real * Dual;
		return *this;
	}

	DualQuaternion& operator*(const float scale)
	{
		Real = Real * scale;
		Dual = Dual * scale;
		return *this;
	}

	inline Quaternion GetRotation() const
	{
		return Real;
	}

	Vector3 GetTranslation() const
	{
		Quaternion conj;
		Real.Conjugate(conj);
		Quaternion t = Dual * 2.0f * conj;
		return Vector3(t.x, t.y, t.z);
	}

	void Normalize()
	{
		float mag = Real.Dot(Real);
		Real *= 1.0f / mag;
		Dual *= 1.0f / mag;
	}

	Matrix ToMatrix() const
	{
		DualQuaternion q = *this;
		q.Normalize();
		Matrix m = Matrix::Identity;
		const float w = q.Real.w;
		const float x = q.Real.x;
		const float y = q.Real.y;
		const float z = q.Real.z;

		// Extract rotational information
		m._11 = w * w + x * x - y * y - z * z;
		m._12 = 2 * x * y + 2 * w * z;
		m._13 = 2 * x * z - 2 * w * y;

		m._21 = 2 * x * y - 2 * w * z;
		m._22 = w * w + y * y - x * x - z * z;
		m._23 = 2 * y * z + 2 * w * x;

		m._31 = 2 * x * z + 2 * w * y;
		m._32 = 2 * y * z - 2 * w * x;
		m._33 = w * w + z * z - x * x - y * y;

		// Extract translation information
		Quaternion conj;
		q.Real.Conjugate(conj);
		Quaternion t = (q.Dual * 2.0f) * conj;
		m._41 = t.x;
		m._42 = t.y;
		m._43 = t.z;

		return m;
	}

	Quaternion Real;
	Quaternion Dual;
};