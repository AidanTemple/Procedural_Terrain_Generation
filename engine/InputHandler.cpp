#include "InputHandler.h"

XINPUT_GAMEPAD controllers[4];

InputHandler::InputHandler()
{
	m_directInput = 0;
	m_keyboard = 0;
	m_mouse = 0;

}

InputHandler::InputHandler(const InputHandler& other)
{

}

InputHandler::~InputHandler()
{

}

bool InputHandler::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;

	// Store the screen size which will be used for positioning the mouse cursor.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Initialize the location of the mouse on the screen.
	m_mouseX = 0;
	m_mouseY = 0;

	// Initialize the main direct input interface.
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the keyboard.
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the keyboard to not share with other programs.
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if(FAILED(result))
	{
		return false;
	}

	// Now acquire the keyboard.
	result = m_keyboard->Acquire();
	if(FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the mouse.
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Set the data format for the mouse using the pre-defined mouse data format.
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if(FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the mouse to share with other programs.
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if(FAILED(result))
	{
		return false;
	}

	// Acquire the mouse.
	result = m_mouse->Acquire();
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

void InputHandler::Shutdown()
{
	// Release the mouse.
	if(m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}

	// Release the keyboard.
	if(m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}

	// Release the main interface to direct input.
	if(m_directInput)
	{
		m_directInput->Release();
		m_directInput = 0;
	}

	return;
}

bool InputHandler::Frame()
{
	bool result;

	// Read the current state of the keyboard.
	result = ReadKeyboard();
	
	if(!result)
	{
		return false;
	}

	// Read the current state of the mouse.
	result = ReadMouse();
	
	if(!result)
	{
		return false;
	}

	// Read the current state of a gamepad.
	result = ReadController();

	if(!result)
	{
		return false;
	}

	// Process the changes in the mouse and keyboard.
	ProcessInput();

	return true;
}

bool InputHandler::ReadKeyboard()
{
	HRESULT result;

	// Read the keyboard device.
	result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	
	if(FAILED(result))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_keyboard->Acquire();
		}
		else
		{
			return false;
		}
	}
		
	return true;
}

bool InputHandler::ReadMouse()
{
	HRESULT result;

	// Read the mouse device.
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	
	if(FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool InputHandler::ReadController()
{
	HRESULT result;

	for(int i = 0; i < 4; i++)
	{
		ZeroMemory(&controllers[i], sizeof(XINPUT_STATE));

		XINPUT_STATE state;
		result = XInputGetState(i, &state);

		if(!result)
		{
			return false;
		}

		if(result == 0)
		{
			controllers[i] = state.Gamepad;
		}
	}

	return true;
}

void InputHandler::ProcessInput()
{
	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;

	// Ensure the mouse location doesn't exceed the screen width or height.
	if(m_mouseX < 0)  { m_mouseX = 0; }
	if(m_mouseY < 0)  { m_mouseY = 0; }
	
	if(m_mouseX > m_screenWidth)  { m_mouseX = m_screenWidth; }
	if(m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }
	
	return;
}

void InputHandler::GetMouseLocation(int& mouseX, int& mouseY)
{
	mouseX = m_mouseX;
	mouseY = m_mouseY;
	return;
}

bool InputHandler::IsEscapePressed()
{
	// Do a bitwise and on the keyboard state to check if the escape key is currently being pressed.
	if(m_keyboardState[DIK_ESCAPE] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsLeftPressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_LEFT] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsRightPressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_RIGHT] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsUpPressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_UP] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsDownPressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_DOWN] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsAPressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_A] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsZPressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_Z] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsWPressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_W] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsSPressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_S] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::IsSpacePressed()
{
	// Do a bitwise and on the keyboard state to check if the key is currently being pressed.
	if(m_keyboardState[DIK_SPACE] & 0x80)
	{
		return true;
	}

	return false;
}

bool InputHandler::XInput_Controller_Found()
{
	XINPUT_CAPABILITIES caps;	
	ZeroMemory(&caps, sizeof(XINPUT_CAPABILITIES));

	XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps);

	if(caps.Type != 0)
	{
		return false;
	}
}

void InputHandler::XInput_Vibrate(int playerIndex, int amount)
{
	XINPUT_VIBRATION vibrate;
	ZeroMemory(&vibrate, sizeof(XINPUT_VIBRATION));

	vibrate.wLeftMotorSpeed = amount;
	vibrate.wRightMotorSpeed = amount;

	XInputSetState(playerIndex, &vibrate);
}

void InputHandler::XInput_Vibrate_LeftMotor(int playerIndex, int amount)
{
	XINPUT_VIBRATION vibrate;
	ZeroMemory(&vibrate, sizeof(XINPUT_VIBRATION));

	vibrate.wLeftMotorSpeed = amount;

	XInputSetState(playerIndex, &vibrate);
}

void InputHandler::XInput_Vibrate_RightMotor(int playerIndex, int amount)
{
	XINPUT_VIBRATION vibrate;
	ZeroMemory(&vibrate, sizeof(XINPUT_VIBRATION));

	vibrate.wRightMotorSpeed = amount;

	XInputSetState(playerIndex, &vibrate);
}