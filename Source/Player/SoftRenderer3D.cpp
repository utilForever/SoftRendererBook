
#include "Precompiled.h"
#include "SoftRenderer.h"
using namespace CK::DDD;

struct Vertex3D
{
public:
	Vertex3D() = default;
	Vertex3D(const Vector4& InPosition) : Position(InPosition) { }
	Vertex3D(const Vector4& InPosition, const LinearColor& InColor) : Position(InPosition), Color(InColor) { }
	Vertex3D(const Vector4& InPosition, const LinearColor& InColor, const Vector2& InUV) : Position(InPosition), Color(InColor), UV(InUV) { }

	Vector4 Position;
	LinearColor Color;
	Vector2 UV;
};

namespace CK::DDD
{
	// ���� ��ȯ �ڵ�
	FORCEINLINE void VertexShader3D(std::vector<Vertex3D>& InVertices, Matrix4x4 InMatrix)
	{
		// ��ġ ���� ���� ����� ������ ��ȯ
		for (Vertex3D& v : InVertices)
		{
			v.Position = InMatrix * v.Position;
		}
	}

	LinearColor colorParam;

	// �ȼ� ��ȯ �ڵ�
	FORCEINLINE LinearColor FragmentShader3D(LinearColor InColor)
	{
		return InColor;
	}
}


void SoftRenderer::DrawGizmo3D()
{
	// �� ����� �׸���
	std::vector<Vertex3D> viewGizmo = {
		Vertex3D(Vector4(Vector3::Zero)),
		Vertex3D(Vector4(Vector3::UnitX * _GizmoUnitLength)),
		Vertex3D(Vector4(Vector3::UnitY * _GizmoUnitLength)),
		Vertex3D(Vector4(Vector3::UnitZ * _GizmoUnitLength)),
	};

	Matrix4x4 viewMatRotationOnly = _GameEngine3.GetMainCamera().GetViewMatrixRotationOnly();
	VertexShader3D(viewGizmo, viewMatRotationOnly);

	// �� �׸���
	Vector2 v0 = viewGizmo[0].Position.ToVector2() + _GizmoPositionOffset;
	Vector2 v1 = viewGizmo[1].Position.ToVector2() + _GizmoPositionOffset;
	Vector2 v2 = viewGizmo[2].Position.ToVector2() + _GizmoPositionOffset;
	Vector2 v3 = viewGizmo[3].Position.ToVector2() + _GizmoPositionOffset;
	_RSI->DrawLine(v0, v1, LinearColor::Red);
	_RSI->DrawLine(v0, v2, LinearColor::Green);
	_RSI->DrawLine(v0, v3, LinearColor::Blue);
}


// ���� ����
void SoftRenderer::Update3D(float InDeltaSeconds)
{
	static float moveSpeed = 100.f;
	static float rotateSpeed = 180.f;

	InputManager input = _GameEngine3.GetInputManager();

	// �÷��̾� ���� ������Ʈ Ʈ�������� ����
	GameObject& player = _GameEngine3.FindGameObject(GameEngine::PlayerKey);
	if (!player.IsNotFound())
	{
		Transform& playerTransform = player.GetTransform();
		Rotator r = playerTransform.GetRotation();
		r.Pitch = input.SpacePressed() ? -90.f : 0.f;
		r.Roll += input.GetZAxis() * rotateSpeed * InDeltaSeconds;
		r.Yaw += input.GetWAxis() * rotateSpeed * InDeltaSeconds;
		playerTransform.SetRotation(r);
	}

	// ����� ���
	if (input.SpacePressed()) { _Show3DGizmo = !_Show3DGizmo; }
}

// ������ ����
void SoftRenderer::Render3D()
{
	// ����� �׸���
	if (_Show3DGizmo)
	{
		DrawGizmo3D();
	}

	Matrix4x4 viewMat = _GameEngine3.GetMainCamera().GetViewMatrix();
	Matrix4x4 perspMat = _GameEngine3.GetMainCamera().GetPerspectiveMatrix();
	const Texture& steveTexture = _GameEngine3.GetMainTexture();

	for (auto it = _GameEngine3.SceneBegin(); it != _GameEngine3.SceneEnd(); ++it)
	{
		const GameObject& gameObject = *it;
		const Mesh& mesh = _GameEngine3.GetMesh(gameObject.GetMeshKey());
		const Transform& transform = gameObject.GetTransformConst();
		Matrix4x4 finalMat = perspMat * viewMat * transform.GetModelingMatrix();

		size_t vertexCount = mesh._Vertices.size();
		size_t indexCount = mesh._Indices.size();
		size_t triangleCount = indexCount / 3;

		// �������� ����� ���� ���ۿ� �ε��� ���۷� ��ȯ
		std::vector<Vertex3D> vertices(vertexCount);
		std::vector<int> indice(mesh._Indices);
		for (int vi = 0; vi < vertexCount; ++vi)
		{
			vertices[vi].Position = Vector4(mesh._Vertices[vi]);
			vertices[vi].UV = mesh._UVs[vi];
		}

		// ���� ��ȯ ����
		VertexShader3D(vertices, finalMat);

		// ȭ�� ũ��� �÷��ֱ�
		for (Vertex3D& v : vertices)
		{
			float invW = 1.f / v.Position.W;
			v.Position.X *= invW;
			v.Position.Y *= invW;
			v.Position.Z *= invW;
			v.Position.W = invW;

			v.Position.X *= (_ScreenSize.X * 0.5f);
			v.Position.Y *= (_ScreenSize.Y * 0.5f);
		}

		for (int ti = 0; ti < triangleCount; ++ti)
		{
			int bi0 = ti * 3, bi1 = ti * 3 + 1, bi2 = ti * 3 + 2;
			Vertex3D& tv0 = vertices[indice[bi0]];
			Vertex3D& tv1 = vertices[indice[bi1]];
			Vertex3D& tv2 = vertices[indice[bi2]];

			Vector3 edge1 = (tv1.Position - tv0.Position).ToVector3();
			Vector3 edge2 = (tv2.Position - tv0.Position).ToVector3();
			Vector3 faceNormal = edge1.Cross(edge2).Normalize();
			if (faceNormal.Z <= 0.f)
			{
				continue;
			}

			// �÷��̾�� �ؽ��Ŀ� ������ ������ ������
			Vector2 minPos(Math::Min3(tv0.Position.X, tv1.Position.X, tv2.Position.X), Math::Min3(tv0.Position.Y, tv1.Position.Y, tv2.Position.Y));
			Vector2 maxPos(Math::Max3(tv0.Position.X, tv1.Position.X, tv2.Position.X), Math::Max3(tv0.Position.Y, tv1.Position.Y, tv2.Position.Y));

			// �����߽���ǥ�� ���� ���� ���ͷ� ��ȯ
			Vector2 u = tv1.Position.ToVector2() - tv0.Position.ToVector2();
			Vector2 v = tv2.Position.ToVector2() - tv0.Position.ToVector2();

			// ���� �и� �� ( uu * vv - uv * uv )
			float udotv = u.Dot(v);
			float vdotv = v.Dot(v);
			float udotu = u.Dot(u);
			float denominator = udotv * udotv - vdotv * udotu;

			// ��ȭ �ﰢ���� ����
			if (Math::EqualsInTolerance(denominator, 0.0f))
			{
				// ��ȭ �ﰢ���̸� �ǳʶ�.
				continue;
			}
			float invDenominator = 1.f / denominator;

			// ȭ����� �� ���ϱ�
			ScreenPoint lowerLeftPoint = ScreenPoint::ToScreenCoordinate(_ScreenSize, minPos);
			ScreenPoint upperRightPoint = ScreenPoint::ToScreenCoordinate(_ScreenSize, maxPos);
			float z0 = tv0.Position.W;
			float z1 = tv1.Position.W;
			float z2 = tv2.Position.W;

			// �ﰢ�� ���� �� ��� ���� �����ϰ� ��ĥ
			for (int x = lowerLeftPoint.X; x <= upperRightPoint.X; ++x)
			{
				for (int y = upperRightPoint.Y; y <= lowerLeftPoint.Y; ++y)
				{
					ScreenPoint fragment = ScreenPoint(x, y);
					Vector2 pointToTest = fragment.ToCartesianCoordinate(_ScreenSize);
					Vector2 w = pointToTest - tv0.Position.ToVector2();
					float wdotu = w.Dot(u);
					float wdotv = w.Dot(v);

					float s = (wdotv * udotv - wdotu * vdotv) * invDenominator;
					float t = (wdotu * udotv - wdotv * udotu) * invDenominator;
					float oneMinusST = 1.f - s - t;
					if (((s >= 0.f) && (s <= 1.f)) && ((t >= 0.f) && (t <= 1.f)) && ((oneMinusST >= 0.f) && (oneMinusST <= 1.f)))
					{
						float z = z0 * oneMinusST + z1 * s + z2 * t;
						float invZ = 1.f / z;

						Vector2 targetUV = (tv0.UV * oneMinusST * z0 + tv1.UV * s * z1 + tv2.UV * t * z2) * invZ;
						_RSI->DrawPoint(fragment, FragmentShader3D(_GameEngine3.GetMainTexture().GetSample(targetUV)));
					}
				}
			}
		}
	}
}
