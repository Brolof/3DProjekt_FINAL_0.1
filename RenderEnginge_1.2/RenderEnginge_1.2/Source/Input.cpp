#include"Input.h"


Input::Input(){

	m_DirInput = 0;
	m_KeyBoard = 0;
	m_Mouse = 0;

}

Input::Input(const Input& other)
{
}

Input::~Input()
{
}

bool Input::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;
	HRESULT result1;
	HRESULT result2;
	HRESULT result3;


	// Store the screen size 
	msWidth = screenWidth;
	msHeight = screenHeight;

	// Initialize the location of the mouse on the screen.
	m_MouseX = 0;
	m_MouseY = 0;

	// Initialize the main direct input interface.
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_DirInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the keyboard.
	result = m_DirInput->CreateDevice(GUID_SysKeyboard, &m_KeyBoard, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Set the data format for the keyboard.
	result = m_KeyBoard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result))
	{
		return false;
	}

	// Set so keyboard interaction is only with our program
	result = m_KeyBoard->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	//Acquire the keyboard
	result = m_KeyBoard->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the mouse.
	result1 = m_DirInput->CreateDevice(GUID_SysMouse, &m_Mouse, NULL);
	if (FAILED(result1))
	{
		return false;
	}

	// Set the data format for the mouse.
	result2 = m_Mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result2))
	{
		return false;
	}

	// Set so mouse interaction is only with our program
	result3 = m_Mouse->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result3))
	{
		return false;
	}

	// Acquire the mouse.
	result = m_Mouse->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void Input::Shutdown(){

	m_Mouse->Unacquire();
	m_Mouse->Release();
	m_Mouse = 0;

	m_KeyBoard->Unacquire();
	m_KeyBoard->Release();
	m_KeyBoard = 0;


	m_DirInput->Release();
	m_DirInput = 0;

}

bool Input::Frame(int sH,int sW){

	//Chekc if any interaction is made, if made go to proccesing
	bool result;

	result = ReadKeyBoard();
	if (result != 1)
	{
		return false;
	}

	result = ReadMouse();
	if (result != 1)
	{
		return false;
	}

	ProcessInputInfo(sH,sW);

	return true;

}

bool Input::ReadKeyBoard(){

	HRESULT result;

	//Read the Keyboard Device
	result = m_KeyBoard->GetDeviceState(sizeof(m_KeyBoardState), (LPVOID)&m_KeyBoardState);
	if (FAILED(result)){
		//Try to get control back
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_KeyBoard->Acquire();
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool Input::ReadMouse(){

	HRESULT result;

	//Read the Mouse Device
	result = m_Mouse->GetDeviceState(sizeof(m_MouseState), (LPVOID)&m_MouseState);
	if (FAILED(result)){
		//Try to get control back
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_Mouse->Acquire();
		}
		else
		{
			return false;
		}
	}
	return true;
}

void Input::ProcessInputInfo(int sW, int sH){

	// Update cursor stuff
	m_MouseX += m_MouseState.lX;
	m_MouseY += m_MouseState.lY;

	// Ensure the mouse location doesn't exceed the screen width or height.
	if (m_MouseX < 0)  { m_MouseX = 0; }
	if (m_MouseY < 0)  { m_MouseY = 0; }

	if (m_MouseX > sW)  { m_MouseX = sW; }
	if (m_MouseY > sH) { m_MouseY = sH; }
}

void Input::GetMouseLocation(int& mouseX, int& mouseY){

	// Return mouse cursor location in x,y
	mouseX = m_MouseX;
	mouseY = m_MouseY;
	return;
}
