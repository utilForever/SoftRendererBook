#pragma once

namespace CK
{
namespace DDD
{

class Camera
{
public:
	Camera() = default;
	~Camera() { }

public:
	Transform& GetTransform() { return _Transform; }
	FORCEINLINE Matrix4x4 GetViewMatrix() const;
	FORCEINLINE Matrix4x4 GetViewMatrixRotationOnly() const;
	FORCEINLINE Matrix4x4 GetPerspectiveMatrix() const;
	void SetLookAtRotation(const Vector3& InTargetPosition);
	void SetAspectRatio(float InAspectRatio) { _AspectRatio = InAspectRatio; }

private:
	Transform _Transform;

	float _FOV = 60.f;
	float _NearZ = 5.5f;
	float _FarZ = 1000.f;
	float _AspectRatio = 1.333f;  // Y�� ����
};

FORCEINLINE Matrix4x4 Camera::GetViewMatrix() const
{
	Vector3 viewX, viewY;
	Vector3 viewZ = -_Transform.GetLocalZ();
	if (viewZ.EqualsInTolerance(Vector3::UnitY) || viewZ.EqualsInTolerance(-Vector3::UnitY))
	{
		viewX = Vector3::UnitZ;
	}
	else
	{
		viewX = Vector3::UnitY.Cross(viewZ).Normalize();
	}
	viewY = viewZ.Cross(viewX);
	Vector3 pos = _Transform.GetPosition();
	return Matrix4x4(
		Vector4(Vector3(viewX.X, viewY.X, viewZ.X), false),
		Vector4(Vector3(viewX.Y, viewY.Y, viewZ.Y), false),
		Vector4(Vector3(viewX.Z, viewY.Z, viewZ.Z), false),
		Vector4(-viewX.Dot(pos), -viewY.Dot(pos), -viewZ.Dot(pos), 1.f)
	);
}

FORCEINLINE Matrix4x4 Camera::GetViewMatrixRotationOnly() const
{
	Vector3 viewX, viewY;
	Vector3 viewZ = -_Transform.GetLocalZ();
	if (viewZ.EqualsInTolerance(Vector3::UnitY) || viewZ.EqualsInTolerance(-Vector3::UnitY))
	{
		viewX = Vector3::UnitZ;
	}
	else
	{
		viewX = Vector3::UnitY.Cross(viewZ).Normalize();
	}
	viewY = viewZ.Cross(viewX);
	return Matrix4x4(
		Vector4(Vector3(viewX.X, viewY.X, viewZ.X), false),
		Vector4(Vector3(viewX.Y, viewY.Y, viewZ.Y), false),
		Vector4(Vector3(viewX.Z, viewY.Z, viewZ.Z), false),
		Vector4::UnitW
	);
}

FORCEINLINE Matrix4x4 Camera::GetPerspectiveMatrix() const
{
	// ���� ���. ���� ���� ������ -1~1
	float invA = 1.f / _AspectRatio;

	float d = 1.f / tanf(Math::Deg2Rad(_FOV) * 0.5f);
	float invNF = 1.f / (_NearZ - _FarZ);
	float k = (_FarZ + _NearZ) * invNF;
	float l = 2.f * _FarZ * _NearZ * invNF;
	return Matrix4x4(
		Vector4::UnitX * invA * d,
		Vector4::UnitY * d,
		Vector4(0.f, 0.f, k, -1.f),
		Vector4(0.f, 0.f, l, 0.f));
}

}
}