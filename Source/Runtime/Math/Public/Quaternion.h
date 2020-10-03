#pragma once

namespace CK
{

struct Quaternion
{
public:
	FORCEINLINE Quaternion() = default;
	FORCEINLINE explicit Quaternion(float InX, float InY, float InZ, float InW) : X(InX), Y(InY), Z(InZ), W(InW) { }
	FORCEINLINE explicit Quaternion(const Vector3& InAxis, float InAngleDegree)
	{
		float sin, cos;
		Math::GetSinCos(sin, cos, InAngleDegree * 0.5f);
		W = cos;
		X = sin * InAxis.X;
		Y = sin * InAxis.Y;
		Z = sin * InAxis.Z;
	}

	FORCEINLINE explicit Quaternion(const Rotator& InRotator)
	{
		float sp, sy, sr;
		float cp, cy, cr;

		Math::GetSinCos(sp, cp, InRotator.Pitch * 0.5f);
		Math::GetSinCos(sy, cy, InRotator.Yaw * 0.5f);
		Math::GetSinCos(sr, cr, InRotator.Roll * 0.5f);

		W = sy * sp * sr + cy * cp * cr;
		X = sy * sr * cp + sp * cy * cr;
		Y = sy * cp * cr - sp * sr * cy;
		Z = -sy * sp * cr + sr * cy * cp;
	}

	FORCEINLINE Quaternion operator*(const Quaternion& InQuaternion) const;
	FORCEINLINE Quaternion operator*=(const Quaternion& InQuaternion);
	FORCEINLINE Vector3 operator*(const Vector3& InVector) const;
	static Quaternion Slerp(const Quaternion &InQuaternion1, const Quaternion &InQuaternion2, float InRatio);

	FORCEINLINE Vector3 RotateVector(const Vector3& InVector) const;
	FORCEINLINE Quaternion Inverse() const { return Quaternion(-X, -Y, -Z, W); }
	FORCEINLINE void Normalize();
	FORCEINLINE Rotator ToRotator() const;
	FORCEINLINE bool IsUnitQuaternion() const
	{
		float size = sqrtf(X * X + Y * Y + Z * Z + W * W);
		if (Math::EqualsInTolerance(size, 1.f))
		{
			return true;
		}
		
		return false;
	}

	FORCEINLINE float RealPart() const { return W; }
	FORCEINLINE Vector3 ImaginaryPart() const { return Vector3(X, Y, Z); }

	static const Quaternion Identity;

public:
	float X = 0.f;
	float Y = 0.f;
	float Z = 0.f;
	float W = 1.f;
};

FORCEINLINE Quaternion Quaternion::operator*(const Quaternion & InQuaternion) const
{
	Quaternion result;
	Vector3 v1(X, Y, Z), v2(InQuaternion.X, InQuaternion.Y, InQuaternion.Z);
	result.W = W * InQuaternion.W - v1.Dot(v2);
	Vector3 v = v2 * W + v1 * InQuaternion.W + v1.Cross(v2);
	result.X = v.X;
	result.Y = v.Y;
	result.Z = v.Z;

	return result;
}

FORCEINLINE Quaternion Quaternion::operator*=(const Quaternion & InQuaternion)
{
	Vector3 v1(X, Y, Z), v2(InQuaternion.X, InQuaternion.Y, InQuaternion.Z);
	W = W * InQuaternion.W - v1.Dot(v2);
	Vector3 v = v2 * W + v1 * InQuaternion.W + v1.Cross(v2);
	X = v.X;
	Y = v.Y;
	Z = v.Z;
	return *this;
}

FORCEINLINE Vector3 Quaternion::operator*(const Vector3& InVector) const
{
	return RotateVector(InVector);
}

FORCEINLINE Quaternion Quaternion::Slerp(const Quaternion & InQuaternion1, const Quaternion & InQuaternion2, float InRatio)
{
	float rawCosValue =
		InQuaternion1.X * InQuaternion2.X +
		InQuaternion1.Y * InQuaternion2.Y +
		InQuaternion1.Z * InQuaternion2.Z +
		InQuaternion1.W * InQuaternion2.W;

	float cosValue = (rawCosValue >= 0.f) ? rawCosValue : -rawCosValue;
	float alpha, beta;
	if (cosValue < 0.9999f)
	{
		const float omega = acosf((cosValue < -1.f) ? -1.f : ((cosValue < 1.f) ? cosValue : 1.f));
		const float invSin = 1.f / sinf(omega);
		alpha = sinf((1.f - InRatio) * omega) * invSin;
		beta = sinf(InRatio * omega) * invSin;
	}
	else
	{
		// ���̰��� ���� ���� ��� ����� ���� ���� ������ ����Ѵ�.  
		alpha = 1.0f - InRatio;
		beta = InRatio;
	}

	beta = (rawCosValue >= 0.f) ? beta : -beta;
	Quaternion result;

	result.X = alpha * InQuaternion1.X + beta * InQuaternion2.X;
	result.Y = alpha * InQuaternion1.Y + beta * InQuaternion2.Y;
	result.Z = alpha * InQuaternion1.Z + beta * InQuaternion2.Z;
	result.W = alpha * InQuaternion1.W + beta * InQuaternion2.W;

	return result;
}

FORCEINLINE Vector3 Quaternion::RotateVector(const Vector3& InVector) const
{
	Vector3 q(X, Y, Z);
	Vector3 t = q.Cross(InVector) * 2.f;
	Vector3 result = InVector + (t * W) + q.Cross(t);
	return result;
}

FORCEINLINE void Quaternion::Normalize()
{
	const float squareSum = X * X + Y * Y + Z * Z + W * W;

	if (squareSum >= KINDA_SMALL_NUMBER)
	{
		const float scale = 1.f / sqrtf(squareSum);

		X *= scale;
		Y *= scale;
		Z *= scale;
		W *= scale;
	}
	else
	{
		*this = Quaternion::Identity;
	}
}

FORCEINLINE Rotator Quaternion::ToRotator() const
{
	Rotator result;
	float sinrCosp = 2 * (W * Z + X * Y);
	float cosrCosp = 1 - 2 * (Z * Z + X * X);
	result.Roll = Math::Rad2Deg(atan2f(sinrCosp, cosrCosp));

	float pitchTest = W * X - Y * Z;
	float asinThreshold = 0.4999995f;
	float sinp = 2 * pitchTest;
	if (pitchTest < -asinThreshold)
	{
		result.Pitch = -90.f;
	}
	else if (pitchTest > asinThreshold)
	{
		result.Pitch = 90.f;
	}
	else
	{
		result.Pitch = Math::Rad2Deg(asinf(sinp));
	}

	float sinyCosp = 2 * (W * Y + X * Z);
	float cosyCosp = 1.f - 2 * (X * X + Y * Y);
	result.Yaw = Math::Rad2Deg(atan2f(sinyCosp, cosyCosp));
	
	return result;
}

}