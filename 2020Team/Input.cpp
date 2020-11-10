#include "Input.h"
#include <cassert>
#include<stdio.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

bool Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	HRESULT result = S_FALSE;

	// DirectInputオブジェクトの生成	
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// キーボードデバイスの生成	
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	result = dinput->CreateDevice(GUID_SysMouse, &devMouse, NULL);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 入力データ形式のセット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard); // 標準形式
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	result = devMouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	result = devMouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	return true;
}

void Input::Update(HWND hwnd)
{
	HRESULT result;

	result = devkeyboard->Acquire();	// キーボード動作開始
	result = devMouse->Acquire();

	// 前回のキー入力を保存
	memcpy(keyPre, key, sizeof(key));

	// キーの入力
	result = devkeyboard->GetDeviceState(sizeof(key), key);
	result = devMouse->GetDeviceState(sizeof(Mouse), &Mouse);

	GetCursorPos(&pos);
	ScreenToClient(hwnd, &pos);
}

bool Input::PushKey(BYTE keyNumber)
{
	// 異常な引数を検出
	assert(0 <= keyNumber && keyNumber <= 256);

	// 0でなければ押している
	if (key[keyNumber]) {
		return true;
	}

	// 押していない
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	// 異常な引数を検出
	assert(0 <= keyNumber && keyNumber <= 256);

	// 前回が0で、今回が0でなければトリガー
	if (!keyPre[keyNumber] && key[keyNumber]) {
		return true;
	}

	// トリガーでない
	return false;
}

bool Input::LeftPush()
{
	if (Mouse.rgbButtons[0]) {
		return true;
	}

	return false;
}

bool Input::RightPush()
{
	if (Mouse.rgbButtons[1]) {
		return true;
	}

	return false;
}

bool Input::mouseX()
{
	if (Mouse.lX) {
		
		return true;
	}
	return false;
}

bool Input::mouseY()
{
	if (Mouse.lY) {
		return true;
	}
	return false;
}
