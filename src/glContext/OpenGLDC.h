#pragma once
#include <Windows.h>
class OpenGLDC
{
public: 
	OpenGLDC();
	~OpenGLDC();

public:
	// 初始化：绑定窗口句柄，创建 OpenGL 上下文
	BOOL Initialize(HWND hWnd);

	// 渲染一帧
	void Render();

	// 响应窗口大小变化
	void Resize(int width, int height);

	// 释放资源
	void Shutdown(); 

public:
	// 鼠标交互
	void OnLButtonDown(int x, int y);
	void OnLButtonUp();
	void OnMouseMove(int x, int y);
	void OnMouseWheel(int delta);

private:
	HWND  m_hWnd;
	HGLRC m_hGLRC;
	HDC   m_hDC;
};

 