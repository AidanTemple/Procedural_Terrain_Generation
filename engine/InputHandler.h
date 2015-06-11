#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

#include <dinput.h>
#include <XInput.h>

extern XINPUT_GAMEPAD controllers[4];

class InputHandler
{
public:
	InputHandler();
	InputHandler(const InputHandler&);
	~InputHandler();

	bool Initialize(HINSTANCE, HWND, int, int);
	bool Frame();
	void Shutdown();

	void GetMouseLocation(int&, int&);

	bool XInput_Controller_Found();
	void XInput_Vibrate(int playerIndex = 0, int amount = 65535);
	void XInput_Vibrate_LeftMotor(int playerIndex = 0, int amount = 65535);
	void XInput_Vibrate_RightMotor(int playerIndex = 0, int amount = 65535);

	bool IsEscapePressed();
	bool IsLeftPressed();
	bool IsRightPressed();
	bool IsUpPressed();
	bool IsDownPressed();
	bool IsAPressed();
	bool IsZPressed();
	bool IsWPressed();
	bool IsSPressed();
	bool IsSpacePressed();

private:
	bool ReadKeyboard();
	bool ReadMouse();
	bool ReadController();

	void ProcessInput();

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	unsigned char m_keyboardState[256];
	DIMOUSESTATE m_mouseState;

	int m_screenWidth, m_screenHeight;
	int m_mouseX, m_mouseY;
};

#endif