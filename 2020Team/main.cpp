#include"WinApi.h"
#include "DirectXSet.h"
#include"Audio.h"
#include "GameScene.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WinApi* winApi = nullptr;
	DirectXSet* dx = nullptr;
	Input* input = nullptr;
	Audio* audio = nullptr;
	GameScene* gameScene = nullptr;


	winApi = new WinApi();
	winApi->CreateGameWindow();

	dx = new DirectXSet();
	dx->Initialize(winApi);

	input = new Input();
	if (!input->Initialize(winApi->GetInstance(), winApi->GetHwnd())) {
		assert(0);
		return 1;
	}
	audio = new Audio();
	if (!audio->Initialize()) {
		assert(0);
		return 1;
	}
	if (!Sprite::StaticInitialize(dx->GetDevice(), WinApi::window_width, WinApi::window_height)) {
		assert(0);
		return 1;
	}
	if(!Object3d::StaticInitialize(dx->GetDevice(), WinApi::window_width, WinApi::window_height)) {
		assert(0);
		return 1;
	}

	gameScene = new GameScene();
	gameScene->Initialize(dx,input);

	while (true) 
	{
		if (winApi->ProcessMessage()) { break; }

		input->Update(winApi->GetHwnd());
		gameScene->Update();

		dx->PreDraw();
		gameScene->Draw();
		dx->PostDraw();
	}
	
	safe_delete(gameScene);
	safe_delete(input);
	safe_delete(audio);
	safe_delete(dx);

	winApi->DeleteGameWIndow();
	safe_delete(winApi);

	return 0;
}