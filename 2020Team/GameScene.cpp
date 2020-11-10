#include "GameScene.h"
#include <cassert>

using namespace DirectX;

GameScene::GameScene() {

}

GameScene::~GameScene()
{
	safe_delete(spriteBG);
	safe_delete(object3d);
}

void GameScene::Initialize(DirectXSet* dx, Input* input)
{
	assert(dx);
	assert(input);

	this->dx = dx;
	this->input = input;

	if (!Sprite::LoadTexture(1, L"Resources/background.png")) {
		assert(0);
		return;
	}
	// 背景スプライト生成
	spriteBG = Sprite::Create(1, { 0.0f,0.0f });

	object3d = Object3d::Create();
	object3d->Update();
}

void GameScene::Update()
{
	if (input->PushKey(DIK_UP) || input->PushKey(DIK_DOWN) || input->PushKey(DIK_RIGHT) || input->PushKey(DIK_LEFT))
	{
		// 現在の座標を取得
		XMFLOAT3 position = object3d->GetPosition();

		// 移動後の座標を計算
		if (input->PushKey(DIK_UP)) { position.y += 0.5f; }
		else if (input->PushKey(DIK_DOWN)) { position.y -= 0.5f; }
		if (input->PushKey(DIK_RIGHT)) { position.x += 0.5f; }
		else if (input->PushKey(DIK_LEFT)) { position.x -= 0.5f; }

		// 座標の変更を反映
		object3d->SetPosition(position);
	}

	if (input->LeftPush())
	{
		XMFLOAT3 position = object3d->GetPosition();
		position.x -= 0.1f;
		object3d->SetPosition(position);
	}

	if (input->RightPush())
	{
		XMFLOAT3 position = object3d->GetPosition();
		position.x += 0.1f;
		object3d->SetPosition(position);
	}

	if (input->mouseX())
	{
		XMFLOAT3 position = object3d->GetPosition();
		position.x += input->mouseVelX() * 0.1f;
		object3d->SetPosition(position);
	}
	
	if (input->mouseY())
	{
		XMFLOAT3 position = object3d->GetPosition();
		position.y -= input->mouseVelY() * 0.1f;
		object3d->SetPosition(position);
	}

	if (input->PushKey(DIK_W) || input->PushKey(DIK_S) || input->PushKey(DIK_D) || input->PushKey(DIK_A))
	{
		if (input->PushKey(DIK_W)) { Object3d::CameraMoveVector({ 0.0f,+1.0f,0.0f }); }
		else if (input->PushKey(DIK_S)) { Object3d::CameraMoveVector({ 0.0f,-1.0f,0.0f }); }
		if (input->PushKey(DIK_D)) { Object3d::CameraMoveVector({ +1.0f,0.0f,0.0f }); }
		else if (input->PushKey(DIK_A)) { Object3d::CameraMoveVector({ -1.0f,0.0f,0.0f }); }
	}
	object3d->Update();
}

void GameScene::Draw()
{
	ID3D12GraphicsCommandList* cmdList = dx->GetCommandList();
	//背面に画像を表示する所
	Sprite::PreDraw(cmdList);
	//spriteBG->Draw();
	Sprite::PostDraw();
	dx->ClearDepthBuffer();

	Object3d::PreDraw(cmdList);

	object3d->Draw();

	Object3d::PostDraw();
	//前面に画像を表示する所
	Sprite::PreDraw(cmdList);
	//sprite1->Draw();
	//sprite2->Draw();
	Sprite::PostDraw();
}