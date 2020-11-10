#pragma once

#include "SafeDelete.h"
#include "DirectXSet.h"
#include <DirectXMath.h>
#include"Object3d.h"
#include"Sprite.h"
#include"Input.h"

class GameScene
{
private:
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	GameScene();
	~GameScene();
	void Initialize(DirectXSet* dx,Input* input);
	void Update();
	void Draw();

private:
	DirectXSet* dx = nullptr;
	Input* input = nullptr;
	Sprite* spriteBG = nullptr;
	Object3d* object3d = nullptr;
};
