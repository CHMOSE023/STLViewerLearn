#pragma once  
#include <Windows.h>
#include "OpenGLDC.h"
class STLWindows
{
public:
	STLWindows();
	~STLWindows(); 

public:
	BOOL Initialize(int width, int height, const wchar_t* title);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT EventProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void Run(); 

public :
	HWND        m_hWnd;
	OpenGLDC*   m_OpenGLDC;
	bool        m_makeReuslt;   
};

 
 