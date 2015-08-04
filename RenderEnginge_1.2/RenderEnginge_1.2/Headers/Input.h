#pragma once
#define DIRECTINPUT_VERSION 0x0800

/////////////
// LINKING INPUT DEVICES//
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
/////////////

#include <windows.h>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>


#include<dinput.h>

using namespace DirectX;



class Input
{
public:
	Input();
	Input(const Input& other);
	~Input();

	bool Initialize(HINSTANCE , HWND , int sWidth, int sHeight);
	void Shutdown();
	bool Frame(int scwh,int sche);

	void GetMouseLocation(int& Mx, int& My);

	bool ReadKeyBoard();
	bool ReadMouse();
	void ProcessInputInfo(int sW, int sH);



	IDirectInput8* m_DirInput;
	IDirectInputDevice8* m_KeyBoard;
	IDirectInputDevice8* m_Mouse;

	unsigned char m_KeyBoardState[256];
	DIMOUSESTATE m_MouseState;
	int msWidth, msHeight;
	int m_MouseX, m_MouseY;

};

