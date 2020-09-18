
#include "Precompiled.h"
#include <random>

using namespace CK::DD;

const std::string GameEngine::QuadMeshKey("SM_Quad");
const std::string GameEngine::PlayerKey("Player");
const std::string GameEngine::SteveTextureKey("Steve.png");

bool GameEngine::Init(const ScreenPoint& InViewportSize)
{
	// 화면 크기의 설정
	_ViewportSize = InViewportSize;

	if (!_InputManager.GetXAxis || !_InputManager.GetYAxis || !_InputManager.GetZAxis || !_InputManager.GetWAxis || !_InputManager.SpacePressed)
	{
		return false;
	}

	if (!LoadResources())
	{
		return false;
	}

	if (!LoadScene())
	{
		return false;
	}

	return true;
}

bool GameEngine::LoadResources()
{
	// 메시 데이터 로딩
	Mesh quadMesh;

	float squareHalfSize = 0.5f;
	int vertexCount = 4;
	int triangleCount = 2;
	int indexCount = triangleCount * 3;

	quadMesh._Vertices = {
		Vector2(-squareHalfSize, -squareHalfSize),
		Vector2(-squareHalfSize, squareHalfSize),
		Vector2(squareHalfSize, squareHalfSize),
		Vector2(squareHalfSize, -squareHalfSize)
	};

	quadMesh._UVs = {
		Vector2(0.125f, 0.75f),
		Vector2(0.125f, 0.875f),
		Vector2(0.25f, 0.875f),
		Vector2(0.25f, 0.75f)
	};

	quadMesh._Indices = {
		0, 2, 1, 0, 3, 2
	};

	_Meshes.insert({ GameEngine::QuadMeshKey , quadMesh });

	// 텍스쳐 로딩
	_MainTexture = Texture(SteveTextureKey);
	if (!GetMainTexture().IsIntialized())
	{
		return false;
	}

	return true;
}

bool GameEngine::LoadScene()
{
	static float playerScale = 20.f;
	static float squareScale = 10.f;

	// 플레이어 게임 오브젝트 생성
	GameObject player(GameEngine::PlayerKey);
	player.SetMesh(GameEngine::QuadMeshKey);
	player.GetTransform().SetScale(Vector2::One * playerScale);
	player.SetColor(LinearColor::Red);
	InsertGameObject(player);

	// 고정 시드로 랜덤하게 생성
	std::mt19937 generator(0);
	std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

	// 100개의 배경 게임 오브젝트 생성
	for (int i = 0; i < 100; ++i)
	{
		char name[64];
		std::snprintf(name, sizeof(name), "GameObject%d", i);
		GameObject newGo(name);
		newGo.GetTransform().SetPosition(Vector2(dist(generator), dist(generator)));
		newGo.GetTransform().SetScale(Vector2::One * squareScale);
		newGo.SetMesh(GameEngine::QuadMeshKey);
		newGo.SetColor(LinearColor::Blue);
		if (!InsertGameObject(std::move(newGo)))
		{
			// 같은 이름 중복이 발생하면 로딩 취소.
			return false;
		}
	}

	return true;
}


// 정렬하면서 삽입하기
bool GameEngine::InsertGameObject(GameObject& InGameObject)
{
	auto it = std::lower_bound(_Scene.begin(), _Scene.end(), InGameObject);
	if (it == _Scene.end())
	{
		_Scene.push_back(std::move(InGameObject));
		return true;
	}

	std::size_t inHash = InGameObject.GetHash();
	std::size_t targetHash = (*it).GetHash();

	if (targetHash == inHash)
	{
		// 중복된 키 발생. 무시.
		return false;
	}
	else if (targetHash < inHash)
	{
		_Scene.insert(it + 1, std::move(InGameObject));
	}
	else
	{
		_Scene.insert(it, std::move(InGameObject));
	}

	return true;
}

GameObject& GameEngine::FindGameObject(const std::string& InName)
{
	auto it = std::lower_bound(_Scene.begin(), _Scene.end(), InName);
	if (it != _Scene.end())
	{
		return (*it);
	}

	return const_cast<GameObject&>(GameObject::NotFound);
}
