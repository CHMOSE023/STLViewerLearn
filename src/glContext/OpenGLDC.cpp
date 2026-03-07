#include <stdio.h>
#include "OpenGLDC.h"
#include <Windows.h> 
#include <glad/glad.h>

OpenGLDC::OpenGLDC():
	m_hDC(nullptr),
	m_hGLRC(nullptr),
	m_hWnd(nullptr)
{
	printf("OpenGLDC::OpenGLDC()\n");
}
OpenGLDC::~OpenGLDC()
{
	printf("OpenGLDC::~OpenGLDC()\n");
}

BOOL OpenGLDC::Initialize(HWND hWnd)
{
	m_hWnd = hWnd; 

    unsigned pixelFormat;
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,              // Not concerned with these.
        0, 0, 0, 0, 0, 0, 0,           // No alpha or accum buffer.
        32,                            // 32-bit depth buffer.
        0, 0,                          // No stencil or aux buffer.
        PFD_MAIN_PLANE,                // Main layer type.
        0,                             // Reserved.
        0, 0, 0                        // Unsupported.
    };

    RECT    rt = { 0, 0, 0, 0 };
    GetClientRect(hWnd, &rt); 
    m_hDC = GetDC(m_hWnd);

    ;

    if ((pixelFormat = ChoosePixelFormat(m_hDC, &pfd)) == 0)
    {
        printf("ChoosePixelFormat failed: %d\n", GetLastError());
        return  false;
    }
    
    if (!SetPixelFormat(m_hDC, pixelFormat, &pfd))
    {
		printf("SetPixelFormat failed: %d\n", GetLastError());
        return  false;
    }
    m_hGLRC = wglCreateContext(m_hDC);
    if (!wglMakeCurrent(m_hDC, m_hGLRC))
    {
        return  false;
    }
      
	gladLoadGL(); // 加载 OpenGL 函数指针

    return  true; 
}

void OpenGLDC::Render()
{
    if (!m_hGLRC) return; 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);


	SwapBuffers(m_hDC);
}

void OpenGLDC::Resize(int width, int height)
{
    if (height == 0)
        height = 1; 

    glViewport(0, 0, width, height);

    Render(); 
}

void OpenGLDC::Shutdown()
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(m_hGLRC);
    ReleaseDC(m_hWnd, m_hDC);

	m_hGLRC = nullptr;
    m_hDC = nullptr;
}

void OpenGLDC::OnLButtonDown(int x, int y)
{
}

void OpenGLDC::OnLButtonUp()
{
}

void OpenGLDC::OnMouseMove(int x, int y)
{
}

void OpenGLDC::OnMouseWheel(int delta)
{
}