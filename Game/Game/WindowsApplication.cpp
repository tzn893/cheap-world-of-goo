#include "WindowsApplication.h"
#include "Common.h"
#include "InputBuffer.h"

using namespace Game;


LRESULT CALLBACK WinProc(HWND,UINT,WPARAM,LPARAM);

namespace Game {
	extern InputBuffer gInput;
};

bool WindowsApplication::initialize() {
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	hinstance = GetModuleHandle(NULL);

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hinstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass";
	wc.lpfnWndProc = WinProc;

	RegisterClassEx(&wc);

	winHandle = CreateWindowEx(0,
		L"WindowClass", L"Homo",
		WS_OVERLAPPEDWINDOW,
		0, 0, config.width, config.height,
		NULL, NULL, hinstance, NULL);

	if (winHandle == NULL) {
		Log("fail to initialize window\n");
		return false;
	}

	ShowWindow(winHandle, SW_SHOW);

	//return  backEnd? backEnd->initialize() : true;
	for (auto iter : moduleList) {
		if (!iter->initialize()) {
			return false;
		}
	}
	return true;
}

bool WindowsApplication::isQuit() {
	return quit;
}

void WindowsApplication::tick() {
	
	MSG msg = {};
	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	else {
		for (auto iter : moduleList) {
			iter->tick();
		}
		
		return;
	}
	quit = msg.message == WM_QUIT;
}

void WindowsApplication::finalize() {
	auto iter = moduleList.rbegin();
	while (iter != moduleList.rend()) {
		(*iter)->finalize();
		iter++;
	}
}

LRESULT CALLBACK WinProc(HWND handle,UINT Msg,
	WPARAM wParam,LPARAM lParam) {

#define GET_X_FROM_LPARAM(lParam) lParam & 0xffff
#define GET_Y_FROM_LPARAM(lParam) lParam >> 16
	int keycode;
	switch (Msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		gInput.BufferWriteKeyDown(InputBuffer::KeyCode::MOUSE_LEFT);
		break;
	case WM_RBUTTONDOWN:
		gInput.BufferWriteKeyDown(InputBuffer::KeyCode::MOUSE_RIGHT);
		break;
	case WM_LBUTTONUP:
		gInput.BufferWriteKeyUp(InputBuffer::KeyCode::MOUSE_LEFT);
		break;
	case WM_RBUTTONUP:
		gInput.BufferWriteKeyUp(InputBuffer::KeyCode::MOUSE_RIGHT);
		break;
	case WM_MOUSEMOVE:
		gInput.BufferWriteMousePosition(GET_X_FROM_LPARAM(lParam),GET_Y_FROM_LPARAM(lParam));
		break;
	case WM_KEYDOWN:
		keycode = wParam - 0x41;
		if (keycode >= 0 && keycode < 26) {
			gInput.BufferWriteKeyDown((InputBuffer::KeyCode)keycode);
		}
		break;
	case WM_KEYUP:
		keycode = wParam - 0x41;
		if (keycode >= 0 && keycode < 26) {
			gInput.BufferWriteKeyUp((InputBuffer::KeyCode)keycode);
		}
		break;
	}

	return DefWindowProc(handle, Msg, wParam, lParam);
}

void Game::WindowsApplication::setTitle(const char* title) {
	SetWindowTextA(winHandle,title);
}