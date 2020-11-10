#pragma once
#include <Windows.h>

class WinApi
{
public:
	static const int window_width = 1280;//ウィンドウの横の大きさ
	static const int window_height = 720;//ウィンドウの縦の大きさ
	static const wchar_t windowClassName[];

public:
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	void CreateGameWindow();

	void DeleteGameWIndow();

	bool ProcessMessage();

	HWND GetHwnd() { return hwnd; }

	HINSTANCE GetInstance() { return wndClass.hInstance; }
private:
	HWND hwnd = nullptr;
	WNDCLASSEX wndClass{};
};
